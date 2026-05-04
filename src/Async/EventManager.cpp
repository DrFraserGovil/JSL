#include <JSL.h>

namespace JSL
{
	std::unique_ptr<EventManager> EventManager::Create(std::string_view id)
	{
		auto r = Watcher::Create(id);
		if (!r)
		{
			// return std::nullopt;
			internal::FatalError("Bad watcher",JSL_LOCATION) << "Could not initialise a watcher: EventManager initialisation failed";
		}
		return std::make_unique<EventManager>(std::move(r.value()));
	}
	EventManager::EventManager(Watcher && w) : Watch(std::move(w))
	{

	}

	void EventManager::SetTextCallback(std::function<void(std::string_view)> callback)
	{
		SetCinCallback(callback);
		SetSocketCallback(callback);
	}
	void EventManager::SetCinCallback(std::function<void(std::string_view)> callback)
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
	void EventManager::SetSocketCallback(std::function<void(std::string_view)> callback)
	{
		callback_socket = callback;

		Watch.SetSocketCallback([this](auto line)
		{
			if (ShutdownCommands.contains((std::string)line))
			{
				this->AddTask(Task::Instruction::Shutdown());
			}
			else
			{
				this->AddTask(Task::Instruction(Task::SOCKET,line));
			}
		});
	}
	void EventManager::SetFileCallback(std::function<void(JSL::FileChange)> callback)
	{
		callback_file = callback;
		Watch.SetInotifyCallback([this](auto file)
		{
			this->AddTask(Task::Instruction(Task::FILE,file));
		});
	}



	void EventManager::Run()
	{
		if (ShutdownCommands.empty())
		{
			ShutdownCommands = {"exit"};
			LOG(WARN) << "No ShutdownCommand(s) assigned. Default inserted to prevent hardlocking:\n" << JSL::MakeString(ShutdownCommands);
		}


		Watch.PrepRun(); //sets LoopRunning to true without relying on a race condition for the thread startup and the Watch.Running loop

		std::thread([this](){
			Watch.Run();
		}).detach();

		while (Watch.Running())
		{
			std::unique_lock lock(Sync);
			CV.wait(lock, [this]{ return !TaskQueue.empty(); });
			lock.unlock();

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

	void EventManager::Exit()
	{
		Watch.Shutdown();
	}

	void EventManager::Process(Task::Instruction job)
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
				internal::FatalError("Unknown jobtype",JSL_LOCATION) << "Could not process job of type " << job.origin;
		}
	}

	void EventManager::AddTask(Task::Instruction job)
	{
		std::lock_guard lock(Sync);
		TaskQueue.push(std::move(job));
		CV.notify_one();
		//lock released when guard goes out of scope
	}
}