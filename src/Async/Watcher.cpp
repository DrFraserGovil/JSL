#include <JSL/Async/Watcher.h>
#include <JSL/internal/error.h>
#include <JSL/Log.h>
#include <set>
#include <thread>

#if defined(_WIN32) || defined(_WIN64)
	#include <io.h>
	#define STDIN_FILENO 0
#else
	#include <unistd.h>
#endif


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
		#if !defined(_WIN32)
				if (INotifyID != -1 && !Callbacks.contains(INotifyID))
				{
					LOG(WARN) << "inotify is watching files, but no callback has been set";
				}
		#endif
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

			int pollresult = 0;
			#if defined(_WIN32)
				// Windows uses WSAPoll for sockets and standard handle loops for directories
				pollresult = WSAPoll(PollCatchers.data(), static_cast<ULONG>(PollCatchers.size()), currentBlock);
			#else
						pollresult = poll(PollCatchers.data(), PollCatchers.size(), currentBlock);
			#endif


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
			#if defined(_WIN32)
						// Windows handles STDIN through a distinct console pipeline handle
						w.fd = reinterpret_cast<UINT_PTR>(GetStdHandle(STD_INPUT_HANDLE));
			#else
						w.fd = STDIN_FILENO;
			#endif

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

		if (INotifyID == reinterpret_cast<watch_id_t>(-1))
		{
			InitialiseINotify();
		}
		
		int wd;
		#if defined(_WIN32)
			static int id_counter = 1;
			wd = id_counter++;
			// Windows directory handling via absolute parent tracking paths
			fs::path targetDir = fs::is_directory(path) ? path : path.parent_path();
			HANDLE hDir = CreateFileW(
				targetDir.c_str(),
				FILE_LIST_DIRECTORY,
				FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
				NULL
			);
			if (hDir != INVALID_HANDLE_VALUE)
			{
				WinDirectoryHandles[wd] = hDir;
			}
		#else
			wd = inotify_add_watch(INotifyID, path.string().c_str(), watchFlags);
		#endif

		WatchMap[wd] = path;
		return wd;
	}

	#if !(defined(_WIN32) || defined(_WIN64))
		alignas(struct inotify_event) thread_local char buffer[4096];
	
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
	#else
		thread_local char win_buffer[4096];
		void Watcher::UpdateFileBatch()
		{
			// Non-blocking query via ReadDirectoryChangesW loop checks
			for (auto const& [wd, hHandle] : WinDirectoryHandles)
			{
				HANDLE hDir = reinterpret_cast<HANDLE>(hHandle);
				DWORD bytesReturned = 0;
				OVERLAPPED overlapped{};

				if (ReadDirectoryChangesW(hDir, win_buffer, sizeof(win_buffer), FALSE,
					FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME,
					&bytesReturned, &overlapped, NULL) || GetLastError() == ERROR_IO_PENDING)
				{
					// If immediate updates aren't available, move to next directory handle frame
					if (bytesReturned == 0) continue;

					auto* notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(win_buffer);
					while (notify)
					{
						std::wstring wname(notify->FileName, notify->FileNameLength / sizeof(wchar_t));
						fs::path changedPath = WatchMap[wd];
						if (!fs::is_directory(changedPath)) {
							if (changedPath.filename() != fs::path(wname).filename()) {
								if (notify->NextEntryOffset == 0) break;
								notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(notify) + notify->NextEntryOffset);
								continue;
							}
						}
						else {
							changedPath /= wname;
						}

						auto report = FileChange(changedPath, notify->Action);
						auto loc = FileCache.find(report);
						if (loc == FileCache.end())
						{
							FileCache.insert(report);
						}

						if (notify->NextEntryOffset == 0) break;
						notify = reinterpret_cast<FILE_NOTIFY_INFORMATION*>(reinterpret_cast<char*>(notify) + notify->NextEntryOffset);
					}
				}
			}
		}
	#endif
	void Watcher::SetInotifyCallback(std::function<void(const FileChange &)> callback)
	{
		if (INotifyID == reinterpret_cast<watch_id_t>(-1))
		{
			InitialiseINotify();
		}

#if !defined(_WIN32)
		if (!Callbacks.contains(INotifyID))
		{
			LOG(DEBUG) << "Filewatcher callback initialised";
			pollfd_t w{};
			w.fd = INotifyID;
			w.events = POLLIN;
			PollCatchers.push_back(w);
			if (BlockNewAdds) FatalError("Bad watch call", JSL_LOCATION) << "Cannot add new watch i:setterations whilst Watcher is running";
		}
#endif

		CachedCallback = [this, callback](auto file)
		{
			callback(file);
		};

		// On Windows, loop directly handles tracking calls instead of poll events
#if defined(_WIN32)
		int fake_win_id = 99999;
		Callbacks[fake_win_id] = [this]()
#else
		Callbacks[INotifyID] = [this]()
#endif
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
			return;
		}

		#if defined(_WIN32)
				if (WinDirectoryHandles.contains(id))
				{
					CloseHandle(reinterpret_cast<HANDLE>(WinDirectoryHandles[id]));
					WinDirectoryHandles.erase(id);
				}
		#else
				inotify_rm_watch(INotifyID, id);
		#endif

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
		#if defined(_WIN32)
				// Set identifier state without blocking system allocations
				INotifyID = reinterpret_cast<watch_id_t>(1);
		#else
				INotifyID = inotify_init1(IN_NONBLOCK);
				if (INotifyID < 0)
				{
					FatalError("Bad inotify", JSL_LOCATION) << "Could not establish inotify process\nReason: " << std::strerror(errno);
				}
		#endif
	}
};