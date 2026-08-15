#include <JSL/Async/Watcher/FileWatcher.h>
#ifdef MACOSMODE
#include <JSL/Log.h>
#include <JSL/internal/error.h>

namespace JSL::Async::Watcher
{

	File::~File()
	{
		Stop();
	}

	void File::AbortStartup(std::string msg)
	{
		// AbortStartup is only ever reached before WorkerThread exists, so Stream (if created at all) is guaranteed to not yet be scheduled/started -- safe to release directly, no Stop/Invalidate dance required here.
		if (Stream)
		{
			FSEventStreamRelease(Stream);
			Stream = nullptr;
		}
		Running.store(false, std::memory_order_release);
		JSL::internal::LibraryError("Failed to start watcher", JSL_LOCATION) << msg;
	}

	void File::CreateShutdownSystem()
	{
		// Unlike Linux/Windows, there's no OS object to create ahead of time here -- the thing Stop() needs to signal (the worker thread's CFRunLoop) doesn't exist until Run() creates it. This just resets the handshake state that Run()/Stop() coordinate through.
		std::lock_guard<std::mutex> lock(RunLoopMutex);
		WatcherRunLoop = nullptr;
		RunLoopReady = false;
	}

	void File::InitialisePlatformWatchers()
	{
		auto abspath = std::filesystem::absolute(RootPath);
		CFStringRef cfPath = CFStringCreateWithCString(nullptr, abspath.c_str(), kCFStringEncodingUTF8);
		CFArrayRef pathsToWatch = CFArrayCreate(nullptr, reinterpret_cast<const void **>(&cfPath), 1, &kCFTypeArrayCallBacks);

		FSEventStreamContext context{};
		context.info = this;

		// kFSEventStreamCreateFlagFileEvents: per-file granularity (added 10.7+) rather than coarse directory-level-only notifications.
		FSEventStreamCreateFlags flags = kFSEventStreamCreateFlagFileEvents | kFSEventStreamCreateFlagNoDefer;

		CFTimeInterval latency = static_cast<CFTimeInterval>(DebounceMs) / 1000.0;
		Stream = FSEventStreamCreate(nullptr, &File::FSEventsCallback, &context, pathsToWatch, kFSEventStreamEventIdSinceNow, latency, flags);

		CFRelease(pathsToWatch);
		CFRelease(cfPath);

		if (!Stream)
		{
			AbortStartup("Could not create the FSEvents stream");
		}
	}

	// FSEvents watches recursively natively -- no per-directory descriptor to add or remove, same reasoning as the Windows backend.
	void File::AddWatch(const std::filesystem::path &, bool) {}
	void File::RemoveWatch(const std::filesystem::path &) {}

	void File::Stop()
	{
		if (!Running.exchange(false))
		{
			return;
		}

		{
			std::unique_lock<std::mutex> lock(RunLoopMutex);
			RunLoopReadyCv.wait(lock, [this] { return RunLoopReady; });
		}

		if (WatcherRunLoop)
		{
			CFRunLoopStop(WatcherRunLoop);
			CFRunLoopWakeUp(WatcherRunLoop);
		}

		if (WorkerThread.joinable())
		{
			if (std::this_thread::get_id() == WorkerThread.get_id())
			{
				WorkerThread.detach();
			}
			else
			{
				WorkerThread.join();
			}
		}

		WatcherRunLoop = nullptr;
		RunLoopReady = false;
	}

	// Some classic c-style pointer shenanigans. This techinically acts on a pointer to the object, which is mutated in as "info"
	void File::FSEventsCallback(ConstFSEventStreamRef, void *info, size_t numEvents, void *eventPaths, const FSEventStreamEventFlags eventFlags[],
		const FSEventStreamEventId[])
	{
		auto *self = static_cast<File *>(info);
		auto **paths = static_cast<char **>(eventPaths);

		bool forceRescan = false;
		bool relevant = false;

		for (size_t i = 0; i < numEvents; ++i)
		{
			FSEventStreamEventFlags flags = eventFlags[i];

			// These flags mean the stream couldn't report reliably for
			// this batch (dropped events, or coalescing forced a
			// directory-level-only report) -- same philosophy as
			// inotify's self-events / Windows' buffer overflow: don't
			// trust a possibly-incomplete payload, force a full rescan.
			if (flags &
				(kFSEventStreamEventFlagMustScanSubDirs | kFSEventStreamEventFlagUserDropped | kFSEventStreamEventFlagKernelDropped |
					kFSEventStreamEventFlagRootChanged))
			{
				forceRescan = true;
				break;
			}

			std::filesystem::path fullPath(paths[i]);
			bool isDir = flags & kFSEventStreamEventFlagItemIsDir;
			if (isDir)
			{
				if (!self->IsBlacklisted(fullPath))
				{
					relevant = true;
					break;
				}
			}
			else if (self->IsWhitelisted(fullPath) && !self->IsBlacklisted(fullPath))
			{
				relevant = true;
				break;
			}
		}

		if (forceRescan || relevant)
		{
			self->ProcessBatch();
			if (self->CriticalErrorState.load(std::memory_order_acquire))
			{
				self->Running.store(false, std::memory_order_release);
				CFRunLoopStop(self->WatcherRunLoop);
			}
		}
	}

	void File::Run()
	{
		WatcherRunLoop = CFRunLoopGetCurrent();

		// Attach observer to signal readiness ONLY once the run loop is active
		CFRunLoopObserverContext ctx{0, this, nullptr, nullptr, nullptr};
		CFRunLoopObserverRef observer = CFRunLoopObserverCreate(
			kCFAllocatorDefault,
			kCFRunLoopEntry,
			false,
			0,
			[](CFRunLoopObserverRef, CFRunLoopActivity, void *info) {
				auto *self = static_cast<File *>(info);
				{
					std::lock_guard<std::mutex> lock(self->RunLoopMutex);
					self->RunLoopReady = true;
				}
				self->RunLoopReadyCv.notify_all();
			},
			&ctx);
		CFRunLoopAddObserver(WatcherRunLoop, observer, kCFRunLoopDefaultMode);

		FSEventStreamScheduleWithRunLoop(Stream, WatcherRunLoop, kCFRunLoopDefaultMode);
		if (!FSEventStreamStart(Stream))
		{
			Running.store(false, std::memory_order_release);
			CFRunLoopRemoveObserver(WatcherRunLoop, observer, kCFRunLoopDefaultMode);
			CFRelease(observer);
			return;
		}

		if (Running.load(std::memory_order_acquire))
		{
			CFRunLoopRun();
		}

		CFRunLoopRemoveObserver(WatcherRunLoop, observer, kCFRunLoopDefaultMode);
		CFRelease(observer);

		FSEventStreamStop(Stream);
		FSEventStreamInvalidate(Stream);
		FSEventStreamRelease(Stream);
		Stream = nullptr;
	}
} // namespace JSL::Async::Watcher
#endif // MACOSMODE
