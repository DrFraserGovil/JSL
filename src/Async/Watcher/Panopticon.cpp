#include <JSL/Async/Watcher/Panopticon.h>
#include <JSL/Log.h>
#include <JSL/internal/error.h>
namespace JSL::Async::Watcher
{
	Panopticon::Panopticon()
	{
	}
	void Panopticon::SetInputCallback(callback fcn, std::optional<std::string> exitString)
	{
		if (IsRunning)
		{
			JSL::internal::LibraryError("In-flight mutation", JSL_LOCATION) << "Cannot modify the cin-callback of a watcher whilst it is running";
		}
		cinCallback = std::move(fcn);
		Input.Initialise([this, exitString = std::move(exitString)](std::string line) {
			{
				std::unique_lock lock(Queue);
				if (exitString && line == *exitString)
				{
					Instructions.push_back({Instruction::Type::SHUTDOWN, "", ""});
				}
				else
				{
					Instructions.push_back({Instruction::Type::CIN, "cin", std::move(line)});
				}
			}
			AwaitingInstruction.notify_one();
		});
	}
	void Panopticon::SetSocketCallback(std::string socketID, callback fcn, bool forceAcquire)
	{

		if (IsRunning)
		{
			JSL::internal::LibraryError("In-flight mutation", JSL_LOCATION) << "Cannot modify the cin-callback of a watcher whilst it is running";
		}
		Socket[socketID] = std::make_unique<Watcher::Socket>(socketID, [this, socketID = socketID](std::string line) {
				{
				std::unique_lock lock(Queue);
				Instructions.push_back({Instruction::Type::SOCKET, socketID, std::move(line)}); 
				} 
				AwaitingInstruction.notify_one(); }, forceAcquire);
		socketCallback[socketID] = std::move(fcn);
	}

	void Panopticon::Start()
	{
		if (IsRunning)
		{
			JSL::internal::LibraryError("Double Start", JSL_LOCATION) << "Cannot re-start a watcher-set whilst it is already running";
		}

		if (Input.Initialised)
		{
			Input.Start();
		}
		for (auto &[_, s] : Socket)
		{
			if (s->Initialised)
			{
				s->Start();
			}
		}

		IsRunning = true;
		while (IsRunning)
		{
			std::deque<Instruction> localQueue;
			{
				std::unique_lock lock(Queue);
				AwaitingInstruction.wait(lock, [&] { return !Instructions.empty(); });
				std::swap(localQueue, Instructions);
			}

			while (!localQueue.empty())
			{
				auto [type, id, msg] = std::move(localQueue.front());
				localQueue.pop_front();

				switch (type)
				{
					case Instruction::Type::SHUTDOWN:
						IsRunning = false;
						localQueue = {}; // flush the local queue
						break;
					case Instruction::Type::CIN:
						cinCallback(msg);
						break;
					case Instruction::Type::SOCKET:
						socketCallback[id](msg);
						break;
					default:
						LOG(ERROR) << "Not yet handled!";
						break;
				}
			}
		}

		Shutdown();
	}

	void Panopticon::Stop()
	{
		{
			std::unique_lock lock(Queue);
			Instructions.push_back({Instruction::Type::SHUTDOWN, "", ""});
		}
		AwaitingInstruction.notify_one();
	}
	void Panopticon::Shutdown()
	{
		if (Input.Initialised)
		{
			LOG(INFO) << "Stopping";
			Input.Stop();
		}
	}

} // namespace JSL::Async::Watcher
