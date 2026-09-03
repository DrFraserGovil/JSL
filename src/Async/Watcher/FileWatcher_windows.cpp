#include <JSL/Async/Watcher/FileWatcher.h>
#ifdef WINMODE
#include <JSL/Log.h>
#include <JSL/internal/error.h>
#include <windows.h>

namespace JSL::Async::Watcher
{
	namespace
	{
		// 64kb limit, so use that
		constexpr DWORD kNotifyBufferSize = 64 * 1024;

		// Deliberately excludes flags set on reads
		constexpr DWORD kNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES |
										FILE_NOTIFY_CHANGE_SIZE | FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;
	} // namespace

	File::~File()
	{
		Stop();
	}

	void File::AbortStartup(std::string msg)
	{
		if (DirectoryHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(DirectoryHandle);
			DirectoryHandle = INVALID_HANDLE_VALUE;
		}
		if (ReadEvent)
		{
			CloseHandle(ReadEvent);
			ReadEvent = nullptr;
		}
		if (ShutdownEvent)
		{
			CloseHandle(ShutdownEvent);
			ShutdownEvent = nullptr;
		}
		Running.store(false, std::memory_order_release);
		JSL::internal::LibraryError("Failed to start watcher", JSL_LOCATION) << msg;
	}

	void File::CreateShutdownSystem()
	{
		// this is signalled by our own mechanisms (either the Panopticon, or an external service, telling the watcher is time to go home
		ShutdownEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr); // manual-reset, initially unsignalled
		if (!ShutdownEvent)
		{
			AbortStartup("Could not create the shutdown event: " + std::to_string(GetLastError()));
		}
	}

	void File::InitialisePlatformWatchers()
	{
		// this is signalled by the OS, telling us that there's data
		ReadEvent = CreateEventW(nullptr, TRUE, FALSE, nullptr); // manual-reset, initially unsignalled
		if (!ReadEvent)
		{
			AbortStartup("Could not create the read-completion event: " + std::to_string(GetLastError()));
		}

		// Create the directory watcher. The flags ensure what we're opening is a directory, that it is time-outcapable and doesn't block
		DirectoryHandle = CreateFileW(RootPath.c_str(), FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
			OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, nullptr);

		if (DirectoryHandle == INVALID_HANDLE_VALUE)
		{
			AbortStartup("Could not open a handle to the watched directory: " + std::to_string(GetLastError()));
		}
		NotifyBuffer.assign(kNotifyBufferSize, 0);
		Overlapped = OVERLAPPED{};
		Overlapped.hEvent = ReadEvent;

		DWORD bytesReturnedSync = 0;
		BOOL ok = ReadDirectoryChangesW(DirectoryHandle, NotifyBuffer.data(), static_cast<DWORD>(NotifyBuffer.size()),
			IsRecursive ? TRUE : FALSE, kNotifyFilter, &bytesReturnedSync, &Overlapped, nullptr);

		if (!ok && GetLastError() != ERROR_IO_PENDING)
		{
			AbortStartup("Could not issue initial directory watch: " + std::to_string(GetLastError()));
		}
	}

	// Windows (via bWatchSubtree) automatically handles recursion, so these are no-ops
	void File::AddWatch(const std::filesystem::path &, bool) {}
	void File::RemoveWatch(const std::filesystem::path &) {}

	void File::Stop()
	{
		if (!Running.exchange(false)) return;

		SetEvent(ShutdownEvent);
		if (WorkerThread.joinable()) WorkerThread.join();

		if (DirectoryHandle != INVALID_HANDLE_VALUE)
		{
			CloseHandle(DirectoryHandle);
			DirectoryHandle = INVALID_HANDLE_VALUE;
		}
		if (ReadEvent)
		{
			CloseHandle(ReadEvent);
			ReadEvent = nullptr;
		}
		if (ShutdownEvent)
		{
			CloseHandle(ShutdownEvent);
			ShutdownEvent = nullptr;
		}
	}

	bool File::notifyCheck(const char *buf, DWORD len)
	{
		DWORD offset = 0;
		while (true)
		{
			auto *record = reinterpret_cast<const FILE_NOTIFY_INFORMATION *>(buf + offset);

			std::wstring wideName(record->FileName, record->FileNameLength / sizeof(WCHAR));
			std::filesystem::path fullPath = RootPath / std::filesystem::path(wideName);

			std::error_code ec;
			// Windows doesn't report IS_DIR like linux, so have a more conservative check
			bool stillExists = std::filesystem::exists(fullPath, ec) && !ec;
			if (stillExists)
			{
				bool isDir = std::filesystem::is_directory(fullPath, ec) && !ec;
				if (isDir)
				{
					if (!IsBlacklisted(fullPath)) return true;
				}
				else if (IsWhitelisted(fullPath) && !IsBlacklisted(fullPath))
				{
					return true;
				}
			}
			else
			{
				return true; // can't classify a vanished entry -- always trigger
			}

			if (record->NextEntryOffset == 0) break; // last record in this buffer
			offset += record->NextEntryOffset;
			if (offset >= len) break;
		}
		return false;
	}

	void File::Run()
	{

		HANDLE waitHandles[2] = {ReadEvent, ShutdownEvent};

		bool pending = false;
		size_t pushBacks = 0;
		bool forceProcess = false;
		bool readPending = true; // is there an outstanding, not-yet-completed ReadDirectoryChangesW call?
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

			if (!readPending)
			{
				ResetEvent(ReadEvent);
				DWORD bytesReturnedSync = 0; // unused for an overlapped call, but the parameter is required
				BOOL ok = ReadDirectoryChangesW(DirectoryHandle, NotifyBuffer.data(), static_cast<DWORD>(NotifyBuffer.size()), IsRecursive ? TRUE : FALSE,
					kNotifyFilter, &bytesReturnedSync, &Overlapped, nullptr);

				if (!ok && GetLastError() != ERROR_IO_PENDING)
				{
					// Couldn't even issue the read. Force a rescan (best
					// effort at recovery) and try again next iteration
					// rather than treating this as immediately fatal.
					forceProcess = true;
					continue;
				}
				readPending = true;
			}

			DWORD timeout = pending ? static_cast<DWORD>(DebounceMs) : INFINITE;
			DWORD waitResult = WaitForMultipleObjects(2, waitHandles, FALSE, timeout);

			if (waitResult == WAIT_TIMEOUT)
			{
				// Debounce window elapsed with no further activity. The
				// read issued above is still outstanding -- left alone,
				// it'll complete whenever the next real change happens.
				forceProcess = true; // serviced at the top of the next iteration
				continue;
			}
			if (waitResult == WAIT_OBJECT_0 + 1) // ShutdownEvent
			{
				CancelIoEx(DirectoryHandle, &Overlapped);
				DWORD dummy = 0;
				GetOverlappedResult(DirectoryHandle, &Overlapped, &dummy, TRUE); // wait for the cancel to actually land
				break;
			}
			if (waitResult == WAIT_OBJECT_0) // ReadEvent -- the outstanding read completed
			{
				DWORD bytesReturned = 0;
				BOOL ok = GetOverlappedResult(DirectoryHandle, &Overlapped, &bytesReturned, FALSE);
				readPending = false;

				if (!ok)
				{
					DWORD err = GetLastError();
					if (err == ERROR_OPERATION_ABORTED) break; // cancelled during shutdown
					// Any other failure: don't trust whatever partial state
					// exists, just force a rescan.
					forceProcess = true;
					continue;
				}
				if (bytesReturned == 0)
				{
					// A zero-length successful result with a buffer this
					// size generally indicates the notification buffer
					// overflowed -- some changes were silently dropped.
					// The event data we DO have can't be trusted to be
					// complete, so force a full rescan instead of parsing it.
					forceProcess = true;
					continue;
				}

				pending = pending || notifyCheck(NotifyBuffer.data(), bytesReturned);
				if (pending)
				{
					++pushBacks;
					if (pushBacks > MaxDebounceBeforeForce)
					{
						pushBacks = 0;
						forceProcess = true;
					}
				}
				continue; // reissue another read now that readPending is false again
			}

			// WAIT_FAILED or an unexpected result -- attempt recovery via rescan
			forceProcess = true;
		}
	}
} // namespace JSL::Async::Watcher
#endif // WINMODE
