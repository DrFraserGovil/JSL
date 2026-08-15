#include <JSL/Async/Watcher/FileWatcher.h>
#ifdef MACOSMODE
#include <CoreFoundation/CoreFoundation.h>
#include <CoreServices/CoreServices.h>
#include <CoreServices/FSEvents.h>
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
		CFStringRef cfPath = CFStringCreateWithCString(nullptr, RootPath.c_str(), kCFStringEncodingUTF8);
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
		if (!Running.exchange(false)) return;

		// Wait until Run() has actually published a run loop
		{
			std::unique_lock<std::mutex> lock(RunLoopMutex);
			RunLoopReadyCv.wait(lock, [this] { return RunLoopReady; });
		}
		CFRunLoopStop(WatcherRunLoop);

		if (WorkerThread.joinable()) WorkerThread.join();

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
		// async guards for the notification that Run has created the loop (so that Stop doesn't destory it before it exists)
		{
			std::lock_guard<std::mutex> lock(RunLoopMutex);
			WatcherRunLoop = CFRunLoopGetCurrent();
			RunLoopReady = true;
		}
		RunLoopReadyCv.notify_all();

		FSEventStreamScheduleWithRunLoop(Stream, WatcherRunLoop, kCFRunLoopDefaultMode);
		if (!FSEventStreamStart(Stream))
		{
			Running.store(false, std::memory_order_release);
			FSEventStreamInvalidate(Stream);
			FSEventStreamRelease(Stream);
			Stream = nullptr;
			return;
		}

		CFRunLoopRun(); // blocks, delivering callbacks, until CFRunLoopStop() is called (by Stop() or FSEventsCallback on critical error)

		// Must happen on this same thread -- FSEventStreamStop/Invalidate
		// are documented as required to run on the thread the stream was
		// scheduled on.
		FSEventStreamStop(Stream);
		FSEventStreamInvalidate(Stream);
		FSEventStreamRelease(Stream);
		Stream = nullptr;
	}
} // namespace JSL::Async::Watcher
#endif // MACOSMODE
