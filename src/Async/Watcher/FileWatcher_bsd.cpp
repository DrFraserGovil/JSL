#include <JSL/Async/Watcher/FileWatcher.h>
#ifdef BSDMODE
#include <JSL/Log.h>
#include <JSL/internal/error.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <poll.h>
#include <sys/event.h>
#include <sys/time.h>
#include <unistd.h>

namespace JSL::Async::Watcher
{
	namespace
	{
		constexpr size_t kEventBufLen = 64;

		// Registered broadly even though we never inspect which flag
		constexpr u_int kWatchFlags = NOTE_WRITE | NOTE_DELETE | NOTE_RENAME | NOTE_EXTEND | NOTE_ATTRIB;
	} // namespace

	File::~File()
	{
		Stop();
	}

	void File::AbortStartup(std::string msg)
	{
		// WatchMap is guaranteed empty here -- AddWatch only ever calls
		// AbortStartup on the very first (root) watch, before any watch
		// has been recorded.
		if (KqueueFd != -1)
		{
			close(KqueueFd);
			KqueueFd = -1;
		}
		if (ShutdownPipe[0] != -1)
		{
			close(ShutdownPipe[0]);
			close(ShutdownPipe[1]);
			ShutdownPipe[0] = ShutdownPipe[1] = -1;
		}
		Running.store(false, std::memory_order_release);
		JSL::internal::LibraryError("Failed to start watcher", JSL_LOCATION) << msg;
	}

	void File::CreateShutdownSystem()
	{
		if (ShutdownPipe[0] != -1)
		{
			close(ShutdownPipe[0]);
			close(ShutdownPipe[1]);
		}
		if (::pipe(ShutdownPipe) == 0)
		{
			fcntl(ShutdownPipe[0], F_SETFL, O_NONBLOCK);
			fcntl(ShutdownPipe[1], F_SETFL, O_NONBLOCK);
		}
		else
		{
			AbortStartup("Could not create a pipe for the FileWatcher");
		}
	}

	void File::InitialisePlatformWatchers()
	{
		KqueueFd = kqueue();
		if (KqueueFd == -1)
		{
			AbortStartup("Could not create a kqueue: " + std::string(strerror(errno)));
		}
	}

	void File::AddWatch(const std::filesystem::path &path, bool isFirstWatch)
	{
		// O_EVTONLY is more efficient, but Darwin only
#ifdef __APPLE__
		int fd = open(path.c_str(), O_EVTONLY);
#else
		int fd = open(path.c_str(), O_RDONLY);
#endif
		if (fd == -1)
		{
			if (!isFirstWatch) return; // TOCTOU: vanished between listing and watching -- safe to skip
			AbortStartup("Could not open a watch on the top level directory: " + std::string(strerror(errno)));
			return;
		}

		struct kevent change{};
		EV_SET(&change, fd, EVFILT_VNODE, EV_ADD | EV_CLEAR, kWatchFlags, 0, nullptr);

		if (kevent(KqueueFd, &change, 1, nullptr, 0, nullptr) == -1)
		{
			close(fd);
			if (!isFirstWatch) return;
			AbortStartup("Could not register a kqueue watch on the top level directory: " + std::string(strerror(errno)));
			return;
		}

		WatchMap[fd] = path;
	}

	void File::RemoveWatch(const std::filesystem::path &path)
	{
		int foundFd = -1;
		for (const auto &[fd, watchedPath] : WatchMap)
		{
			if (watchedPath == path)
			{
				foundFd = fd;
				break;
			}
		}
		if (foundFd == -1) return;

		// Closing the fd automatically drops its kqueue registration -- no separate "remove" call needed, unlike inotify_rm_watch.
		close(foundFd);
		WatchMap.erase(foundFd);
	}

	void File::Stop()
	{
		if (!Running.exchange(false)) return;

		uint8_t signal = 1;
		(void)::write(ShutdownPipe[1], &signal, sizeof(signal));
		if (WorkerThread.joinable()) WorkerThread.join();

		for (auto &[fd, path] : WatchMap) close(fd);
		WatchMap.clear();

		if (KqueueFd != -1) close(KqueueFd);
		KqueueFd = -1;
		close(ShutdownPipe[0]);
		close(ShutdownPipe[1]);
		ShutdownPipe[0] = ShutdownPipe[1] = -1;
	}

	void File::Run()
	{
		struct pollfd fds[2] = {};
		fds[0].fd = KqueueFd;
		fds[0].events = POLLIN;
		fds[1].fd = ShutdownPipe[0];
		fds[1].events = POLLIN;

		bool pending = false;
		size_t pushBacks = 0;
		bool forceProcess = false;

		struct kevent eventBuf[kEventBufLen];
		struct timespec zeroTimeout{0, 0};

		while (Running.load(std::memory_order_acquire))
		{
			if (forceProcess)
			{
				pushBacks = 0;
				ProcessBatch();
				pending = false;
				forceProcess = false;
				if (CriticalErrorState)
				{
					Running.store(false, std::memory_order_release);
					break;
				}
				continue;
			}

			int timeout = pending ? DebounceMs : -1;
			int ret = ::poll(fds, 2, timeout);

			if (ret == 0)
			{
				// debounce window elapsed with no further activity
				forceProcess = true;
				continue;
			}
			if (ret < 0)
			{
				if (errno == EINTR) continue;
				break;
			}
			if (fds[1].revents & POLLIN) break; // shutdown requested

			if (fds[0].revents & POLLIN)
			{
				// Non-blocking drain: we don't care which fd(s) fired or why: just go rescan
				int n = kevent(KqueueFd, nullptr, 0, eventBuf, static_cast<int>(kEventBufLen), &zeroTimeout);
				if (n > 0)
				{
					pending = true;
					++pushBacks;
					if (pushBacks > MaxDebounceBeforeForce)
					{
						pushBacks = 0;
						forceProcess = true;
					}
				}
			}
		}
	}
} // namespace JSL::Async::Watcher
#endif // BSDMODE
