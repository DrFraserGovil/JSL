#include <JSL/Async/Watcher.h>
#include <JSL/internal/error.h>
#include <poll.h>
#include <JSL/Log.h>
#include <set>
#include <thread>


namespace JSL::Event
{
	using namespace JSL::internal;
	namespace fs = std::filesystem;
	std::optional<Watcher> Watcher::Create(std::string_view socketName,double timeout, bool forceAcquire)
	{
		auto r = Antenna::Create(socketName,forceAcquire);		
		if (!r)
		{
			return std::nullopt;
		}
		r->SetTimeout(timeout);

		return Watcher(std::move(r.value()));
	}

	Watcher::Watcher(Antenna && socketIn) : Receiver(std::move(socketIn))
	{
		SetMaxRuntime(-1);	
		Sender = Antenna::Hotline(Receiver);
	}

	using clock = std::chrono::steady_clock;
	using namespace std::literals::chrono_literals;
	void Watcher::PrepRun()
	{
		BlockNewAdds = true;
		InitialiseTime = clock::now();
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

		*LoopRunning = true;
		
	}
	void Watcher::Run()
	{
		PrepRun();
		clock::time_point lastPing = clock::now(); 
		while (*LoopRunning)
		{
			int currentBlock = BlockingTime;
		
			if (AwaitingDebounce)
			{
				//wow this is verbose
				int remainingMilliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(Wakeup - clock::now()).count();
				currentBlock = std::max(0, (int)std::min(currentBlock, remainingMilliseconds));
			}

			int pollresult = poll(PollCatchers.data(),PollCatchers.size(),currentBlock);
			if (AwaitingDebounce && clock::now() >= Wakeup)
			{
				for (auto file : FileCache)
				{
					CachedCallback(file);
				}
				FileCache.clear();
				AwaitingDebounce = false;
			}

			bool withinTime = (clock::now() - lastPing) < Timeout || Timeout <= 0ms; 
			if (!withinTime)
			{
				LOG(INFO) << "Session has timed out.";
				*LoopRunning = false;
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
			*LoopRunning = *LoopRunning && Receiver.IsActive();
			lastPing = clock::now();
		}
		BlockNewAdds = false;
	}

	void Watcher::SetSocketTimeout(double seconds)
	{
		Receiver.SetTimeout(seconds);
		Sender.Timeout = seconds;
	
	}


	void Watcher::Message(std::string_view msg)
	{
		//spawning a detached thread prevents internal deadlock as the message awaits a reponse that can't come until the Watcher hits its next poll cycle
		std::thread([this, m = std::string(msg)]()
		{
			Sender.Send(m);
		}).detach();
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
			if (BlockNewAdds) FatalError("Bad watch call",JSL_LOCATION) << "Cannot add new watch iterations whilst Watcher is running";

		}
		
		Callbacks.insert_or_assign(STDIN_FILENO,[this,callback](){
			std::string line;
			if (std::getline(std::cin, line))
			{
				callback(line);
			}
		});

	}

	void Watcher::Shutdown()
	{
		Receiver.Deactivate();
		*LoopRunning = false;
	}

	std::set<fs::path> Watcher::GetWatchedFiles()
	{
		std::set<fs::path> out;
		for (auto [k,v] : WatchMap)
		{
			out.insert(v);
		}
		return out;
	}
	std::chrono::steady_clock::duration Watcher::GetRuntime()
	{
		if (InitialiseTime)
		{
			return clock::now() - InitialiseTime.value(); 
		}
		else
		{
			return std::chrono::nanoseconds(0);
		}
	}

	void Watcher::SetSocketCallback(std::function<void(std::string_view)> callback)
	{
		int fd = Receiver.GetDescriptor();
		if (!Callbacks.contains(fd))
		{
			LOG(DEBUG) << "Socket callback initialised";
			pollfd w{};
			w.fd = fd;
			w.events = POLLIN;
			
			PollCatchers.push_back(w);
			if (BlockNewAdds) FatalError("Bad watch call",JSL_LOCATION) << "Cannot add new watch iterations whilst Watcher is running";
		}
		Callbacks.insert_or_assign(fd, [this,callback]()
		{
			auto msg = Receiver.Read();
			
			if (!msg.empty())
			{
				callback(msg);
			}
		});
	}

	
	int Watcher::Watch(fs::path path,uint32_t watchFlags)
	{
		if (!fs::exists(path))
		{
			FatalError("File Does not Exist",JSL_LOCATION) << "Cannot watch " << path.string() << " as it does not exist";
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

	void Watcher::UpdateFileBatch()
	{
		int length = read(INotifyID, buffer,sizeof(buffer));
		int i = 0;
		
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

				auto loc = FileCache.find(report);
				if (loc == FileCache.end())
				{
					FileCache.insert(report);
				}
				else
				{
					auto old = FileCache.extract(loc);
					old.value().Mask |= report.Mask;
					FileCache.insert(std::move(old));
				}
			}
		

			i += sizeof(struct inotify_event) + event->len;
		}
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
			if (BlockNewAdds) FatalError("Bad watch call",JSL_LOCATION) << "Cannot add new watch iterations whilst Watcher is running";

		}
		
		CachedCallback = [this,callback](auto file)
		{
			callback(file);
		};

		Callbacks[INotifyID] = [this,callback]()
		{
			if (!AwaitingDebounce)
			{
				AwaitingDebounce = true;
				Wakeup = clock::now() + std::chrono::milliseconds(DebounceMS);
			}
			UpdateFileBatch();
		};
	}
	void Watcher::SetDebounce(size_t milliseconds)
	{
		DebounceMS = milliseconds;
	}
	void Watcher::Unwatch(int id)
	{
		if (!WatchMap.contains(id))
		{
			return; //unwatching a nonwatched file is not an error
		}

		inotify_rm_watch(INotifyID,id);
		// if (attempt <0)
		// {
		// 	FatalError("Bad inotify_rm",JSL_LOCATION) << "Could not unwatch " << WatchMap[id].string() << " inotify has crashed";
		// }
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
			FatalError("Bad inotify", JSL_LOCATION) << "Could not establish inotify process\nReason: " << std::strerror(errno);
		}		
	}
};