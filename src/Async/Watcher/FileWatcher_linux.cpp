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
	}

	void File::AbortStartup(std::string msg)
	{
		if (InotifyFd != -1)
		{
			::close(InotifyFd);
			InotifyFd = -1;
		}
		if (ShutdownPipe[0] != -1)
		{
			::close(ShutdownPipe[0]);
			::close(ShutdownPipe[1]);
			ShutdownPipe[0] = ShutdownPipe[1] = -1;
		}
		Running.store(false, std::memory_order_release);
		JSL::internal::LibraryError("Failed to start watcher", JSL_LOCATION) << msg;

		// declared [[noreturn]]
	}
	void File::InitialisePlatformWatchers()
	{
		/// initialise the inotify system
		InotifyFd = inotify_init1(IN_NONBLOCK | IN_CLOEXEC);
		if (InotifyFd == -1)
		{
			Running.store(false, std::memory_order_release);
			AbortStartup("inotify_init1 failed with message: " + (std::string)std::strerror(errno));
		}
	}
	void File::CreateShutdownSystem()
	{
		if (ShutdownPipe[0] != -1)
		{
			::close(ShutdownPipe[0]); // close any previously opened pipes
			::close(ShutdownPipe[1]);
		}
		/// Create the shutdown messager
		if (::pipe(ShutdownPipe) == 0) // opens a new pipe, and saves the values into ShutdownPipe
		{
			::fcntl(ShutdownPipe[0], F_SETFL, O_NONBLOCK);
			::fcntl(ShutdownPipe[1], F_SETFL, O_NONBLOCK);
		}
		else
		{
			Running.store(false, std::memory_order_release);
			AbortStartup("Could not create a shutdown pipe for the InputWatcher");
		}
	}

	void File::Stop()
	{
		// same stop pattern as is in InputWatcher
		if (!Running.exchange(false)) return;

		uint8_t signal = 1;
		[[maybe_unused]] auto tmp = (::write(ShutdownPipe[1], &signal, sizeof(signal)));
		if (WorkerThread.joinable())
			WorkerThread.join();

		for (auto &[wd, dir] : WatchMap)
		{
			inotify_rm_watch(InotifyFd, wd);
		}
		WatchMap.clear();

		if (InotifyFd != -1) ::close(InotifyFd);
		InotifyFd = -1;
		::close(ShutdownPipe[0]);
		ShutdownPipe[0] = -1;
		::close(ShutdownPipe[1]);
		ShutdownPipe[1] = -1;
	}

	void File::AddWatch(const std::filesystem::path &dir, bool isFirstWatch)
	{
		if (isFirstWatch)
		{
			WatchMap.clear();
		}
		// create the inotify instance for this directory
		int wd = inotify_add_watch(InotifyFd, dir.c_str(), kWatchMask);
		if (wd == -1)
		{
			if (!isFirstWatch) return; // TOCTOU, safe to skip

			// else: didn't establish watch on top level (that's
			AbortStartup("Coult not establish a watch on the top level directory, with error " + std::string(std::strerror(errno)));
		}

		// store it so we can work out later who said what
		WatchMap[wd] = dir;
	}
	void File::RemoveWatch(const std::filesystem::path &dir)
	{
		int wd = -1;
		for (auto &[tmpwd, path] : WatchMap)
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
		auto timePendingActivated = std::chrono::steady_clock::now();
		bool forceProcess = false;
		while (Running.load(std::memory_order_acquire))
		{
			int timeout = pending ? DebounceMs : -1;
			if (forceProcess) timeout = 0;
			int ret = ::poll(fds, 2, timeout);

			if (ret == 0 || forceProcess) // timed out
			{
				ProcessBatch();
				pending = false;
				forceProcess = false;
				if (CriticalErrorState)
				{
					Running = false;
					return;
				}
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
					bool wasPending = pending;
					// if there was data present, determine if it requires a reprocessing pass
					pending = pending || inotifyCheck(buf.data(), n);

					// debounce logic
					if (pending && !wasPending)
					{
						// i.e. if this is a new debounce sequence
						timePendingActivated = std::chrono::steady_clock::now();
					}
					else
					{
						auto elapsed = std::chrono::steady_clock::now() - timePendingActivated;
						int msElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count();
						if (msElapsed > MaxDebounceBeforeForce * DebounceMs)
						{
							forceProcess = true;
						}
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
			if (it == WatchMap.end()) continue; // stale/already-removed watch; or a SELF_DELETE which we catch in other ways

			if (event->len == 0)
			{
				return true; // len == 0 occurs for IN_DELETE_SELF or IN_MOVE_SELF, which is the case for a dir we're watching being moved/deleted -- which is an automatic rescan trigger
			}
			std::filesystem::path fullPath = it->second / event->name;

			if (event->mask & IN_ISDIR)
			{
				if (!IsBlacklisted(fullPath)) return true; // we only *blacklist* directories
			}
			else if (IsWhitelisted(fullPath) && !IsBlacklisted(fullPath)) // files need to pass both white and blacklists
			{
				return true;
			}
		}
		return false;
	}

} // namespace JSL::Async::Watcher
#endif // LINUXMODE
