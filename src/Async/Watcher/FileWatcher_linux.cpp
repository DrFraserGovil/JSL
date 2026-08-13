#include <JSL/Async/Watcher/FileWatcher.h>
#ifdef LINUXMODE
#include <JSL/Log.h>
#include <JSL/internal/error.h>
#include <cerrno>
#include <climits>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/inotify.h>
#include <unistd.h>

namespace JSL::Async::Watcher
{
	namespace
	{
		constexpr uint32_t kWatchMask =
			IN_CREATE | IN_DELETE | IN_MODIFY | IN_MOVED_FROM | IN_MOVED_TO | IN_CLOSE_WRITE | IN_ATTRIB | IN_DELETE_SELF | IN_MOVE_SELF;
		constexpr size_t kEventBufLen = 64 * (sizeof(struct inotify_event) + NAME_MAX + 1);
	} // namespace

	File::~File()
	{
		Stop();
		::close(ShutdownPipe[0]);
		::close(ShutdownPipe[1]);
	}

	void File::Start()
	{
		if (!Initialised)
		{
			JSL::internal::LibraryError("Invalid state", JSL_LOCATION) << "File watcher started before Initialise() was called";
		}
		if (Running.exchange(true)) return;

		/// Create the shutdown messager
		if (ShutdownPipe[0] != -1)
		{
			::close(ShutdownPipe[0]); // close any previously opened pipes
			::close(ShutdownPipe[1]);
		}
		if (::pipe(ShutdownPipe) == 0) // opens a new pipe, and saves the values into ShutdownPipe
		{
			::fcntl(ShutdownPipe[0], F_SETFL, O_NONBLOCK);
			::fcntl(ShutdownPipe[1], F_SETFL, O_NONBLOCK);
		}
		else
		{
			JSL::internal::LibraryError("Invalid pipe", JSL_LOCATION) << "Could not create a pipe for the InputWatcher";
		}

		/// initialise the inotify system
		InotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
		if (InotifyFd == -1)
		{
			Running.store(false, std::memory_order_release);
			JSL::internal::LibraryError("Failed to start watcher", JSL_LOCATION) << "inotify_init1 failed: " << std::strerror(errno);
		}

		InitialSnapshot();
		WatchMap.clear();
		AddWatch(RootPath);
		for (auto &dir : PreviousDirs)
		{
			AddWatch(dir);
		}

		WorkerThread = std::thread(&File::Run, this);
	}

	void File::Stop()
	{
		// same stop pattern as is in InputWatcher
		if (!Running.exchange(false)) return;

		uint8_t signal = 1;
		(void)::write(ShutdownPipe[1], &signal, sizeof(signal));
		if (WorkerThread.joinable()) WorkerThread.join();

		for (auto &[wd, dir] : WatchMap)
		{
			inotify_rm_watch(InotifyFd, wd);
		}
		WatchMap.clear();

		if (InotifyFd != -1) ::close(InotifyFd);
		InotifyFd = -1;
	}

	void File::AddWatch(const std::filesystem::path &dir)
	{
		LOG(INFO) << "Watching " << dir;
		// create the inotify instance for this directory
		int wd = inotify_add_watch(InotifyFd, dir.c_str(), kWatchMask);
		if (wd == -1) return; // e.g. TOCTOU: dir vanished between listing and watching

		// store it so we can work out later who said what
		WatchMap[wd] = dir;
	}
	void File::RemoveWatch(const std::filesystem::path &dir)
	{
		int wd = -1;
		for (auto [tmpwd, path] : WatchMap)
		{
			if (path == dir)
			{
				wd = tmpwd;
				break;
			}
		}
		if (wd == -1) return;

		inotify_rm_watch(InotifyFd, wd);
		WatchMap.erase(wd);
	}

	void File::Run()
	{
		// set up two polls to watch -- the inotify itself, and the shutdown pipe
		struct pollfd fds[2] = {};
		fds[0].fd = InotifyFd;
		fds[0].events = POLLIN;
		fds[1].fd = ShutdownPipe[0];
		fds[1].events = POLLIN;

		bool pending = false;
		std::vector<char> buf(kEventBufLen);
		size_t pushBacks = 0;
		while (Running.load(std::memory_order_acquire))
		{
			int timeout = pending ? DebounceMs : -1;
			int ret = ::poll(fds, 2, timeout);

			if (ret == 0) // timed out
			{
				pushBacks = 0;
				ProcessBatch();
				pending = false;
				if (CriticalErrorState) { return; }
				continue;
			}
			if (ret < 0)
			{
				if (errno == EINTR) continue; // spurious wakeup
				break;						  // bigger error!
			}

			if (fds[1].revents & POLLIN) break; // shutdown requested

			if (fds[0].revents & POLLIN)
			{
				// read in the data from the inotify pipe
				ssize_t n = ::read(InotifyFd, buf.data(), buf.size());
				if (n > 0)
				{
					// if there was data present, determine if it requires a reprocessing pass
					pending = inotifyCheck(buf.data(), n);
					if (pending)
					{
						++pushBacks;
					}
					// quick and dirty circuit breaker in case we get a continual stream of events that stunlock us into a debounce loop
					if (pushBacks > MaxDebounceBeforeForce)
					{
						pushBacks = 0;
						ProcessBatch();
						pending = false;
						if (CriticalErrorState) { return; }
					}
				}
			}
		}
	}

	bool File::inotifyCheck(const char *buf, ssize_t len)
	{
		ssize_t offset = 0;
		while (offset < len)
		{
			// lovely c-style pointer hackery to convert the charbuf into a pointer to an event
			auto *event = reinterpret_cast<const struct inotify_event *>(buf + offset);
			offset += static_cast<ssize_t>(sizeof(struct inotify_event) + event->len);

			auto it = WatchMap.find(event->wd);
			if (it == WatchMap.end()) continue; // stale/already-removed watch

			std::filesystem::path fullPath = it->second / event->name;
			if (IsWhitelisted(fullPath) && !IsBlacklisted(fullPath))
			{
				return true;
			}
		}
		return false;
	}

} // namespace JSL::Async::Watcher
#endif // LINUXMODE
