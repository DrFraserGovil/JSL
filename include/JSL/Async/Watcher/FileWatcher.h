#pragma once
#if defined(_WIN32) || defined(_WIN64)
#include "../Socket/CrossPlatformHeaders.h"
#include <windows.h>

#define WINMODE
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || defined(__DragonFly__) || (defined(__APPLE__) && defined(__MACH__))
#define BSDMODE
#else
#define LINUXMODE
#endif
#include <JSL/IO/Directory.h>
#include <atomic>
#include <filesystem>
#include <functional>
#include <map>
#include <regex>
#include <set>
#include <string>
#include <thread>
namespace JSL::Async::Watcher
{
	//! Describes the change that was applied to an object
	enum class ChangeType
	{
		//! The object exists on disk, and did not at the time of the last scan
		Create,
		//! The object does not exist on disk, and did at the time of the last scan
		Delete,
		//! The object exists (and did exist before), but has changed its contents
		Modify
	};

	//! Describes the type of object that has been modified
	enum class ObjectType
	{
		//! The object is a file with metadata that can be extracted
		File,
		//! The object is a directory
		Directory,
		//! The object is something else (a symlink, pipes etc), or a file which cannot have its metadata read
		Other,
	};

	//! Describes a change to a single filesystem unit
	struct FileChange
	{
		//! The path to the object (relative to the RootPath of the Watcher)
		std::filesystem::path Path;

		//! The type of change which occurred (renames are counted as a sequential delete-then-create)
		ChangeType Change;

		//! The type of object
		ObjectType Object;

		//! Defines an ordering based on the path, so that we can put this object in a set
		auto operator<=>(const FileChange &other) const
		{
			return Path <=> other.Path;
		}
	};

	//! @brief An Async::Watcher class that fires a callback function whenever a file or directory is changed
	//! @details Unlike the other Watchers, the implementation of this class is *highly* platform specific.
	class File
	{
	  public:
		using callback = std::function<void(std::set<FileChange>)>;

		//! @brief Blank constructor: creates an object in an uninitialised state
		File();

		/*! @brief Creates a Watcher which calls the provided function on batches of files which are changed
			@param path The directory to be watched
			@param recursive If true, all child directories are watched. Otherwise, only the top level directory is watched.
			@param fcn A callback function which processes each batch of file changes. Additional arguments should be provided via a capturing lambda. The callback is executed on a parallel, non-blocking thread, so any data races must be manually handled by the user.
			@throws std::runtime_error if the associated Async::Socket constructor throws an error
		  */
		explicit File(std::filesystem::path path, bool recursive, callback fcn);

		/*! @brief A delayed initialisation (for blank-constructed watchers), or sets a new callback function
			@param path The directory to be watched
			@param recursive If true, all child directories are watched. Otherwise, only the top level directory is watched.
			@param fcn A callback function which processes each batch of file changes. Additional arguments should be provided via a capturing lambda. The callback is executed on a parallel, non-blocking thread, so any data races must be manually handled by the user.
			@throws std::runtime_error if the associated Async::Socket constructor throws an error
		  */
		void Initialise(std::filesystem::path path, bool recursive, callback fcn);

		/*! @brief Adds in a new pattern to the whitelist. Only files which match this pattern will appear in the batchs (this has no effect on directories)
			@param whiteGlob A posix-style glob
		*/
		void AddWhiteList(std::string whiteGlob);

		/*! @brief Adds in a new pattern to the blacklist. Files or directories which match this pattern will be omitted from the file batching
			@details On POSIX systems, blacklisted directories are never descended into, and their events never reach the poll, making the system much more fficient
			@param blackGlob A posix-style glob
		*/
		void AddBlackList(std::string blackGlob);

		//! @brief Calls Stop() and cleans up any resources
		~File();

		//! @brief A non-blocking function call which starts the process of watching cin. The callback is always executed on this parallel thread.
		//! @throws std::runtime_error If the object was not initialised with a callback function or target socket
		void Start();

		//! @brief Stops the parallel thread and joins it, halting any further callbacks.
		//! @details There may be a small gap in between the invocation of this function and the actual stoppage, where more events may occur.
		void Stop();

		friend class Panopticon;

	  private:
		//! Generates the initial snapshot and watches used at initialisation time
		void InitialSnapshot();

		//! @brief The function loop used by the worker thread, where the actual watching happens.
		void Run();

		//! @brief Places a call to JSL::IO::Directory::Snapshot to make a record of the current status of the watched direcotry
		//! @returns The current snapshot, which can be compared to previous ones
		JSL::IO::Directory TakeSnapshot();

		/*! @brief Compares the most recent snapshot of the watched directory to the previous one, and batch the results
			@returns A collated list of the changes between the two snapshots
		 */
		std::set<FileChange> ComputeDiff();

		//! Performs the diff-computation on filesystem objects with metadata (enabling a Modify result if the size or modification time differ)
		void ComputeDiff_Meta(std::set<FileChange> &output, const JSL::IO::Directory &currentSnapshot);

		//! Performs the diff-computation on filesystem objects without metadata (so only Create/Delete events can be seen)
		void ComputeDiff_Path(std::set<FileChange> &output, const JSL::IO::Directory &currentSnapshot);

		/*! @brief Checks if the path meets the whitelist criteria
			@param relativePath A candidate path (relative to the RootPath)
			@returns true if whitelisted (or if no whitelist was set), false if not
		 */
		bool IsWhitelisted(const std::filesystem::path &relativePath) const;

		/*! @brief Checks if the path meets the blacklist criteria
		  @param relativePath A candidate path (relative to the RootPath)
		  @returns true if blacklisted, false if not
		  */
		bool IsBlacklisted(const std::filesystem::path &relativePath) const;

		//! @brief Set to True when Initialise() is called (or when the non-blank constructor runs).
		bool Initialised = false;

		//! @brief The storage location for the callback functions given during Initialisation; this is the function called on each batch
		callback Callback;

		//! @brief This is the value which is set to false to indicate to the workers to exit their loops
		std::atomic<bool> Running{false};

		//! @brief The asynchronous thread on which the Run() function is executed, after Start() is called
		std::thread WorkerThread;

		//! The storage of the blacklist as glob-strings
		std::vector<std::string> Blacklist;
		//! The storage of the whitelist as glob-strings
		std::vector<std::string> Whitelist;
		//! The result of calling IO::multiGlob on Blacklist; a single regex which matches all of the Blacklist globs
		std::regex combinedBlacklist;
		//! The result of calling IO::multiGlob on Whitelist; a single regex which matches all of the Whitelist globs
		std::regex combinedWhitelist;

		//! Retains if the Initialiser/constructor were called in recursive mode (and thus if newly created directories need to be watched)
		bool IsRecursive = false;

		//! The Path of the top-level directory being watched. All paths are given relative to this value
		std::filesystem::path RootPath;

		//! The set of FileMetadata reported in the most recent Directory::Snapshot
		std::set<JSL::IO::FileMetadata> PreviousMeta;

		//! The set of Directories reported in the most recent Directory::Snapshot
		std::set<std::filesystem::path> PreviousDirs;

		//! The set of not-a-file-not-a-directory reported in the most recent Directory::Snapshot
		std::set<std::filesystem::path> PreviousOthers;

		//! The debounce time between the first filechange and when we report our batch
		//! This gives the system time to settle (editors etc. often to a whole bunch of save-then-rename in bursts)
		static constexpr int DebounceMs = 20;

		//! If a continual stream of events occurs, it can stunlock the debouncer: this is the maximum number of times the debouncer can be hit before we force a batch to form
		static constexpr int MaxDebounceBeforeForce = 10;

		//! IF this is true, the Worker thread will quit
		std::atomic<bool> CriticalErrorState = false;

		/*! @brief Attach a watcher module to the direcotry (for POSIX systems)
			@details If the watch fails to initialise, this fails silently (i.e. the direcotry will not report any events)
			@param dir The direcotry to be watched
			@param isFirstWatch Used to signal that this is the watch being applied to the top-level object.
			@throws std::runtime_error If isFirstWatch is true, and a watch cannot be initialised
		 */
		void AddWatch(const std::filesystem::path &dir, bool isFirstWatch = false);

		//! @brief Remove the directory from any records about being watched (POSIX only)
		//! @param dir The directory to be removed
		void RemoveWatch(const std::filesystem::path &dir);

		//! @brief Calls ComputeDiff and then does Add/Remove watch, before activating the Callback
		void ProcessBatch();

		//! @brief A highly platform-specific function which initialises the resources required for the file watching
		void InitialisePlatformWatchers();

		//! @brief Create the platform specific mechanisms for interrupting the read() calls and shutting down the asynchronous thread
		void CreateShutdownSystem();

		//! A collator function for releasing resources (i.e. file descriptors) in the event that the constructor is about to throw
		[[noreturn]] void AbortStartup(std::string msg);
#ifdef LINUXMODE

		//! @brief Checks if the changes recorded meet the filtering requirements, and if so, signal that a re-cache is needed
		bool inotifyCheck(const char *buf, ssize_t len);

		//! The file descriptor of the inotify process
		int InotifyFd{-1};

		//! A pipe which can be signalled in the normal poll/fds system, activating shutdown
		int ShutdownPipe[2]{-1, -1};

		//! A map connecting inotify-created file descriptors and the direcotry they map to
		std::map<int, std::filesystem::path> WatchMap; // watch descriptor -> absolute directory path

		//! A flag which tells indicates that linux does per-directory watches (not per-file)
		bool PlatformWatchFiles = false;
#endif
#ifdef WINMODE
		//! @brief Checks if the changes recorded meet the filtering requirements, and if so, signal that a re-cache is needed
		bool notifyCheck(const char *buf, DWORD len);

		//! The windows equivalent of the inotify fd; a handle to the watcher process
		HANDLE DirectoryHandle{INVALID_HANDLE_VALUE};

		//! The windows equivalent of the ShutdownPipe; a handle which can be signalled to activate shutdown
		HANDLE ShutdownEvent{nullptr};

		//! A windows counterpart to the pollers, this is signalled to indicate that the DirectoryHandle has
		HANDLE ReadEvent{nullptr};

		//! A flag which tells indicates that windows does per-directory watches (not per-file)
		bool PlatformWatchFiles = false;
#endif
#ifdef BSDMODE
		//! A flag which tells indicates that bsd/kqueue does per-file watches (not per-directory)
		bool PlatformWatchFiles = true;

		//! The direct BSD-analogue to the inotify fd; a file descriptor to the kqueue process
		int KqueueFd{-1};

		//! A pipe which can be signalled in the normal poll/fds system, activating shutdown
		int ShutdownPipe[2]{-1, -1};

		//! A map connecting inotify-created file descriptors and the direcotry they map to
		std::map<int, std::filesystem::path> WatchMap; // watch descriptor -> absolute directory path
#endif
		//! rule of 3 deletion
		File(const File &) = delete;
		//! rule of 3 deletion
		File &operator=(const File &) = delete;
	};
} // namespace JSL::Async::Watcher
