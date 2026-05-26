#include <JSL/Async/ManagerBase.h>
#include <JSL/Display/Log.h>
#include <JSL/Strings/Serialisers.h>
#include <JSL/internal/error.h>
namespace JSL::Event::internal
{
	using namespace JSL::internal;
	Watcher MakeWatcher(std::string_view id)
	{
		auto r = Watcher::Create(id);
		if (!r)
		{
			FatalError("Bad watcher",JSL_LOCATION) << "Could not initialise a watcher: EventManager initialisation failed";
		}
		return std::move(r.value());
	}


	HandlerBase::HandlerBase(std::string_view id): Watch(std::move(MakeWatcher(id)))
	{
	}
	HandlerBase::HandlerBase(Watcher & watch) : Watch(std::move(watch)){
	}

	void HandlerBase::SetTextCallback(std::function<void(std::string_view)> callback)
	{
		SetCInCallback(callback);
		SetSocketCallback(callback);
	}
	void HandlerBase::SetCInCallback(std::function<void(std::string_view)> callback)
	{
		callback_cin = callback;

		Watch.SetCInCallback([this](auto line)
		{
			if (ShutdownCommands.contains(line))
			{
				this->AddTask(Task::Instruction::Shutdown());
			}
			else
			{
				this->AddTask(Task::Instruction(Task::CIN,line));
			}
		});
	}
	void HandlerBase::SetSocketCallback(std::function<void(std::string_view)> callback)
	{
		callback_socket = callback;

		Watch.SetSocketCallback([this](auto line)
		{
			if (ShutdownCommands.contains(line))
			{
				this->AddTask(Task::Instruction::Shutdown());
			}
			else
			{
				this->AddTask(Task::Instruction(Task::SOCKET,line));
			}
		});
	}
	void HandlerBase::SetInotifyCallback(std::function<void(JSL::Event::FileChange)> callback)
	{
		callback_file = callback;
		Watch.SetInotifyCallback([this](auto file)
		{
			this->AddTask(Task::Instruction(Task::FILE,file));
		});
	}

	void HandlerBase::Exit()
	{
		Watch.Shutdown();
		CV.notify_all();
	}

	void HandlerBase::InitialiseRun()
	{
		if (ShutdownCommands.empty())
		{
			ShutdownCommands = {"exit"};
			LOG(WARN) << "No ShutdownCommand(s) assigned. Default inserted to prevent hardlocking:\n" << JSL::String::makeFrom(ShutdownCommands);
		}


		Watch.PrepRun(); //sets LoopRunning to true without relying on a race condition for the thread startup and the Watch.Running loop

		std::thread([this](){
			Watch.Run();
		}).detach();

		Running = true;
	}

	void HandlerBase::Process(Task::Instruction job)
	{
		switch (job.origin)
		{
			case Task::SHUTDOWN:
				Exit();
				break;
			case Task::CIN:
				callback_cin(std::get<std::string>(job.payload));
				break;
			case Task::SOCKET:
				callback_socket(std::get<std::string>(job.payload));
				break;
			case Task::FILE:
				callback_file(std::get<FileChange>(job.payload));
				break;
			default:
				FatalError("Unknown jobtype",JSL_LOCATION) << "Could not process job of type " << job.origin;
		}
	}

	void HandlerBase::Run()
	{
		InitialiseRun(); // common functions
		LOG(INFO) << "Running " << Running;

		while (Watch.Running())
		{
			std::unique_lock lock(Sync);
			CV.wait(lock, [this]{ return !TaskQueue.empty() || !Watch.Running(); });
			lock.unlock();

			Synchroniser(); //does nothing in serial, in parallel forces everybody to sync up and not take any new jobs until we've done this

			while (true) //loop until we hit the break 
			{
				lock.lock();

				if (TaskQueue.empty())
				{
					lock.unlock();
					break;
				}
				auto job = TaskQueue.front();
				TaskQueue.pop();
				lock.unlock();
				Process(job); //potentially long 

			}
		}
	}
	void HandlerBase::ProtectAdd()
	{
		if (!Running)
		{
			FatalError("Add before Run",JSL_LOCATION) << "Cannot add a task to an EventHandler before it is running";
		}
	}

	JSL::Event::Watcher * HandlerBase::GetWatcher()
	{
		return & Watch;
	}
	
}