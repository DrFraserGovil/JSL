#include <JSL/Async/Watcher/Panopticon.h>
#include <JSL/Log.h>
#include <JSL/internal/error.h>
namespace JSL::Async::Watcher
{
	void Panopticon::SetInputCallback(strCallBack fcn, std::optional<std::string> exitString, bool overwriteExisting)
	{
		if (IsRunning)
		{
			JSL::internal::LibraryError("In-flight mutation", JSL_LOCATION) << "Cannot modify the cin-callback of a watcher whilst it is running";
		}

		if (cinCallback)
		{
			if (!overwriteExisting)
			{
				JSL::internal::LibraryError("Callback overwrite", JSL_LOCATION) << "A callback is already registered to the Input Watcher. To prevent this error, run with overwriteExisting = true";
			}
			if (exitString)
			{
				LOG(WARN) << "Cannot set a new exitString value when overriding a socket callback.";
			}
		}
		else
		{
			InputTracker.Initialise([this, exitString = std::move(exitString)](std::string line) {
				{
					std::unique_lock lock(Queue);
					if (exitString && line == *exitString)
					{
						Instructions.push_back({internal::Instruction::Type::SHUTDOWN, "", ""});
					}
					else
					{
						Instructions.push_back({internal::Instruction::Type::CIN, "cin", std::move(line)});
					}
				}
				AwaitingInstruction.notify_one();
			});
		}
		cinCallback = std::move(fcn);
	}
	void Panopticon::SetSocketCallback(std::string socketID, strCallBack fcn, bool forceAcquire, bool overwriteExisting)
	{

		if (IsRunning)
		{
			JSL::internal::LibraryError("In-flight mutation", JSL_LOCATION) << "Cannot modify the socket-callback of a watcher whilst it is running";
		}
		if (SocketTracker.contains(socketID))
		{
			if (!overwriteExisting)
			{
				JSL::internal::LibraryError("Callback overwrite", JSL_LOCATION) << "A callback is already registered to the socket " << socketID << ". To prevent this error, run with overwriteExisting = true";
			}
		}
		else
		{
			SocketTracker[socketID] = std::make_unique<Watcher::Socket>(socketID, [this, socketID = socketID](std::string line) {
					{
					std::unique_lock lock(Queue);
					Instructions.push_back({internal::Instruction::Type::SOCKET, socketID, std::move(line)}); 
					} 
					AwaitingInstruction.notify_one(); }, forceAcquire);
		}
		socketCallback[socketID] = std::move(fcn);
	}
	void Panopticon::SetFileBatchCallback(std::string watchedDirectory, bool recursive, batchCallBack fcn, bool overwriteExisting)
	{

		if (IsRunning)
		{
			JSL::internal::LibraryError("In-flight mutation", JSL_LOCATION) << "Cannot modify the file-callback of a watcher whilst it is running";
		}
		if (FileTracker.contains(watchedDirectory))
		{
			if (!overwriteExisting)
			{
				JSL::internal::LibraryError("Callback overwrite", JSL_LOCATION) << "A callback is already registered to the directory " << watchedDirectory << ". To prevent this error, run with overwriteExisting = true";
			}
		}
		else
		{
			FileTracker[watchedDirectory] = std::make_unique<Watcher::File>(watchedDirectory, recursive, [this, id = watchedDirectory](std::set<FileChange> batch) {
					{
					std::unique_lock lock(Queue);
					Instructions.push_back({internal::Instruction::Type::FILE, id, std::move(batch)}); 
					} 
					AwaitingInstruction.notify_one(); });
		}
		fileCallback[watchedDirectory] = std::move(fcn);
	}
	void Panopticon::SetSingleFileCallback(std::string watchedDirectory, bool recursive, fileCallBack fcn, bool overwriteExisting)
	{
		SetFileBatchCallback(watchedDirectory, recursive, [func = fcn](auto batch) {
			for (auto &file : batch)
			{
				func(file);
			} }, overwriteExisting);
	}

	void Panopticon::Start()
	{
		if (IsRunning)
		{
			JSL::internal::LibraryError("Double Start", JSL_LOCATION) << "Cannot re-start a watcher-set whilst it is already running";
		}

		if (InputTracker.Initialised)
		{
			InputTracker.Start();
		}
		for (auto &[_, s] : SocketTracker)
		{
			if (s->Initialised)
			{
				s->Start();
			}
		}
		for (auto &[_, f] : FileTracker)
		{
			if (f->Initialised)
			{
				f->Start();
			}
		}

		IsRunning = true;
		while (IsRunning)
		{
			std::deque<internal::Instruction> localQueue;
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
					case internal::Instruction::Type::SHUTDOWN:
						IsRunning = false;
						localQueue = {}; // flush the local queue
						break;
					case internal::Instruction::Type::CIN:
						cinCallback(std::get<std::string>(msg));
						break;
					case internal::Instruction::Type::SOCKET:
						socketCallback[id](std::get<std::string>(msg));
						break;
					case internal::Instruction::Type::FILE:
						fileCallback[id](std::get<std::set<FileChange>>(msg));
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
			Instructions.push_back({internal::Instruction::Type::SHUTDOWN, "", ""});
		}
		AwaitingInstruction.notify_one();
	}
	void Panopticon::Shutdown()
	{
		if (InputTracker.Initialised)
		{
			InputTracker.Stop();
		}
		for (auto &[_, s] : SocketTracker)
		{
			if (s->Initialised) s->Stop();
		}
		for (auto &[_, f] : FileTracker)
		{
			if (f->Initialised) f->Stop();
		}
	}

} // namespace JSL::Async::Watcher
