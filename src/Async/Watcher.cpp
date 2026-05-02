#include <JSL/Async.h>
#include <JSL/internal/error.h>
#include <poll.h>
#include <JSL/Display/Log.h>
#include <set>
#include <thread>

namespace JSL
{
	std::optional<Watcher> Watcher::Create(std::string_view socketName,double timeout, bool forceAcquire)
	{
		auto r = Socket::Antenna(socketName,timeout,forceAcquire);
		if (!r)
		{
			return std::nullopt;
			// internal::FatalError("Bad Socket",JSL_LOCATION) << "Could not launch Watcher process: socket could not initialise";
		}
		// auto s = Socket::Broadcaster(socketName);
		// if (!s)
		// {
		// 	internal::FatalError("Bad Socket",JSL_LOCATION) << "Could not launch Watcher process: socket could not initialise";
		// }
		return Watcher(std::move(r.value()));//,std::move(s.value()));
	}

	Watcher::Watcher(Socket && socketIn) : Receiver(std::move(socketIn))
	{
		SetMaxRuntime(-1);	
	}

	using clock = std::chrono::steady_clock;
	using namespace std::literals::chrono_literals;
	void Watcher::Run()
	{
		BlockNewAdds = true;
		
		//do some checks to see if running is a good idea
		if (INotifyID != -1 && !Callbacks.contains(INotifyID))
		{
			LOG(WARN) << "inotify is watching files,  but no callback has been set";
		}
		if (Callbacks.size() == 0)
		{
			LOG(WARN) << "No callbacks have been set, so no blocking can occur";
			return;
		}

		
		bool Running = true;
		clock::time_point launch = clock::now(); 
		while (Running)
		{
			int currentBlock = BlockingTime;
			if (!DebounceQueue.empty())
			{
				//wow this is verbose
				int remainingMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(DebounceQueue.front().ActivationTime - clock::now()).count();
				currentBlock = std::max(0, (int)std::min(currentBlock, remainingMilliseconds));
			}
			int pollresult = poll(PollCatchers.data(),PollCatchers.size(),currentBlock);

			while (!DebounceQueue.empty() && clock::now() >= DebounceQueue.front().ActivationTime)
			{
				auto & obj = DebounceQueue.front();
				CurrentDebounce.erase(obj.ID);
				obj.Function();
				DebounceQueue.pop();
			}

			bool withinTime = (clock::now() - launch) < Timeout || Timeout <= 0ms; 
			if (!withinTime)
			{
				LOG(INFO) << "Session has timed out.";
				Running = false;
			}
			if (pollresult <= 0)
			{
				if (errno == EINTR || pollresult == 0) continue; //no wakeup, just a timeout

				LOG(WARN) << "Watcher poll system recieved erroneous shutdown command: exiting loop";
				break;
			}

			for (auto & tracker : PollCatchers)
			{
				if (tracker.revents & POLLIN)
				{
					Callbacks[tracker.fd]();
				}
			}


			//do the runtime checks
			Running &= Receiver.IsActive();
		}
		BlockNewAdds = false;
	}

	void Watcher::SetSocketTimeout(double seconds)
	{
		Receiver.SetTimeout(seconds);
		if (Sender)
		{
			Sender.value().SetTimeout(seconds);
		}
	}


	void Watcher::Message(std::string_view msg)
	{
		if (!Sender)
		{
			Sender = Socket::Broadcaster(Receiver.GetPath().string());
			Receiver.Sync(Sender.value());
		}

		Sender->Send(msg);
		// return Sender.SendAndReply(msg);
	}
	
	void Watcher::SetMaxRuntime(double minutes)
	{
		if (minutes < 0)
		{
			Timeout = -1ms; //no timeout
		}
		else
		{
			Timeout = std::chrono::duration_cast<clock::duration>(std::chrono::duration<double, std::ratio<60>>(minutes));
		}
	}

	void Watcher::SetBlockingTime(double seconds)
	{
		if (seconds < 0)
		{
			BlockingTime = -1; //sets to full block
		}
		else
		{
			BlockingTime = seconds * 1000;
		}	
	}

	void Watcher::SetCInCallback(std::function<void(std::string_view)> callback)
	{
		if (!Callbacks.contains(STDIN_FILENO))
		{
			LOG(DEBUG) << "stdcin callbak initialised";
			pollfd w;
			w.fd = STDIN_FILENO;
			w.events = POLLIN;
			PollCatchers.push_back(w);
			if (BlockNewAdds) internal::FatalError("Bad watch call",JSL_LOCATION) << "Cannot add new watch iterations whilst Watcher is running";

		}
		
		Callbacks.insert_or_assign(STDIN_FILENO,[this,callback](){
			std::string line;
			if (std::getline(std::cin, line))
			{
				callback(line);
			}
		});

	}

	void Watcher::SetSocketCallback(std::function<void(std::string_view)> callback)
	{
		int fd = Receiver.Descriptor();
		if (!Callbacks.contains(fd))
		{
			LOG(DEBUG) << "Socket callback initialised";
			pollfd w{};
			w.fd = fd;
			w.events = POLLIN;
			
			PollCatchers.push_back(w);
			if (BlockNewAdds) internal::FatalError("Bad watch call",JSL_LOCATION) << "Cannot add new watch iterations whilst Watcher is running";
		}
		Callbacks.insert_or_assign(fd, [this,callback]()
		{
			auto [msg,client] = Receiver.ReadAndReply();
			
			if (!msg.empty())
			{
				callback(msg);
				client.Send("Acknowledge " + msg);
			}
		});
	}

	namespace fs = std::filesystem;
	int Watcher::Watch(fs::path path,uint32_t watchFlags)
	{
		if (!fs::exists(path))
		{
			internal::FatalError("File Does not Exist",JSL_LOCATION) << "Cannot watch " << path.string() << " as it does not exist";
		}

		if (INotifyID == -1)
		{
			InitialiseINotify();
		}

		int wd = inotify_add_watch(INotifyID,path.string().c_str(),watchFlags);
		WatchMap[wd] = path;
		return wd;
	}


	alignas(struct inotify_event) thread_local char buffer[4096]; //cstyle buffer array

	std::set<FileChange> Watcher::GetFileBatch()
	{
		int length = read(INotifyID, buffer,sizeof(buffer));
		int i = 0;
		std::set<FileChange> batch;
		
		while (i < length)
		{
			struct inotify_event* event = (struct inotify_event*)&buffer[i];

			int id =event->wd;

			//if this is false, we got caught in a race condition where id was removed whilst this event was still in the buffer
			//in that case, just ignore the file
			if (WatchMap.contains(id))
			{
				fs::path path = WatchMap[id];
				if (event->len > 0)
				{
					path/= event->name;
				}
				
				auto report = FileChange(path, event->mask);

				auto loc = batch.find(report);
				if (loc == batch.end())
				{
					batch.insert(report);
				}
				else
				{
					auto old = batch.extract(loc);
					old.value().Mask |= report.Mask;
					batch.insert(std::move(old));
				}
			}
		

			i += sizeof(struct inotify_event) + event->len;
		}
		return batch;
	}

	void Watcher::SetInotifyCallback(std::function<void(const FileChange &)> callback)
	{
		if (INotifyID == -1)
		{
			InitialiseINotify();
		}
		if (!Callbacks.contains(INotifyID))
		{
			LOG(DEBUG) << "Filewatcher callback initialised";
			pollfd w;
			w.fd = INotifyID;
			w.events = POLLIN;
			PollCatchers.push_back(w);
			if (BlockNewAdds) internal::FatalError("Bad watch call",JSL_LOCATION) << "Cannot add new watch iterations whilst Watcher is running";

		}
		
		Callbacks[INotifyID] = [this,callback]()
		{

			if (!CurrentDebounce.contains(INotifyID))
			{
				CurrentDebounce.insert(INotifyID);
				DebounceQueue.emplace(INotifyID,clock::now() + std::chrono::milliseconds(DebounceMS),
				[this,callback]()
				{
					auto batch = GetFileBatch();
					for (auto & file : batch)
					{
						callback(file);
					}
				}
			);
			}
		};
	}

	void Watcher::Unwatch(int id)
	{
		if (!WatchMap.contains(id))
		{
			return; //unwatching a nonwatched file is not an error
		}

		int attempt = inotify_rm_watch(INotifyID,id);
		if (attempt <0)
		{
			internal::FatalError("Bad inotify_rm",JSL_LOCATION) << "Could not unwatch " << WatchMap[id].string() << " inotify has crashed";
		}
		WatchMap.erase(id);
	}
	void Watcher::Unwatch(fs::path file)
	{
		for (auto & [id,path] : WatchMap)
		{
			if (file == path)
			{
				Unwatch(id);
				return;
			}
		}

		//no error if reach here: unwatching a nonwatched file is not an error
	}
	void Watcher::InitialiseINotify()
	{
		INotifyID = inotify_init();
		if (INotifyID < 0)
		{
			internal::FatalError("Bad inotify", JSL_LOCATION) << "Could not establish inotify process\nReason: " << std::strerror(errno);
		}		
	}
};