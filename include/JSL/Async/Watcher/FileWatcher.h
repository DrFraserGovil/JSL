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
	enum class ChangeType
	{
		Create,
		Delete,
		Modify
	};
	enum class ObjectType
	{
		File,
		Directory,
		Other,
	};
	struct FileChange
	{
		std::filesystem::path Path;
		ChangeType Change;
		ObjectType Object;
		auto operator<=>(const FileChange &other) const
		{
			return Path <=> other.Path;
		}
	};

	class File
	{
	  public:
		using callback = std::function<void(std::set<FileChange>)>;

		File();
		explicit File(std::filesystem::path path, bool recursive, callback fcn);

		void Initialise(std::filesystem::path path, bool recursive, callback fcn);

		void AddWhiteList(std::string whiteGlob);

		void AddBlackList(std::string blackGlob);

		~File();

		void Start();
		void Stop();

		File(const File &) = delete;
		File &operator=(const File &) = delete;

		friend class Panopticon;

	  private:
		void InitialSnapshot();
		void Run(); // platform-specific: owns the trigger + debounce loop

		JSL::IO::Directory TakeSnapshot();
		// Compares two snapshots and produces the batch of changes between
		// them, applying Whitelist as a post-filter on the result. Both
		// snapshots must already be relative-to-RootPath (Directory's own
		// convention), so no path rebasing happens here.
		std::set<FileChange> ComputeDiff();
		void ComputeDiff_Meta(std::set<FileChange> &output, const JSL::IO::Directory &currentSnapshot);
		void ComputeDiff_Path(std::set<FileChange> &output, const JSL::IO::Directory &currentSnapshot);

		bool IsWhitelisted(const std::filesystem::path &relativePath) const;
		bool IsBlacklisted(const std::filesystem::path &relativePath) const;

		bool Initialised = false;
		callback Callback;
		std::atomic<bool> Running{false};
		std::thread WorkerThread;

		std::vector<std::string> Blacklist;
		std::vector<std::string> Whitelist;
		std::regex combinedBlacklist;
		std::regex combinedWhitelist;

		bool IsRecursive = false;
		std::filesystem::path RootPath;

		std::set<JSL::IO::FileMetadata> PreviousMeta;
		std::set<std::filesystem::path> PreviousDirs;
		std::set<std::filesystem::path> PreviousOthers;

		static constexpr int DebounceMs = 20;
		static constexpr int MaxDebounceBeforeForce = 10;
		std::atomic<bool> CriticalErrorState = false;
		void AddWatch(const std::filesystem::path &dir, bool isFirstWatch = false);
		void RemoveWatch(const std::filesystem::path &dir);
		void ProcessBatch();
		void InitialisePlatformWatchers();
		void CreateShutdownSystem();
		[[noreturn]] void AbortStartup(std::string msg);
#ifdef LINUXMODE

		//! @brief Checks if the changes recorded meet the filtering requirements, and if so, signal that a re-cache is needed
		bool inotifyCheck(const char *buf, ssize_t len);
		int InotifyFd{-1};
		int ShutdownPipe[2]{-1, -1};
		std::map<int, std::filesystem::path> WatchMap; // watch descriptor -> absolute directory path
		bool PlatformWatchFiles = false;
#endif
#ifdef WINMODE
		bool notifyCheck(const char *buf, DWORD len);
		HANDLE DirectoryHandle{INVALID_HANDLE_VALUE};
		HANDLE ShutdownEvent{nullptr};
		HANDLE ReadEvent{nullptr};
		bool PlatformWatchFiles = false;
#endif
#ifdef BSDMODE
		bool PlatformWatchFiles = true;
		int KqueueFd{-1};
		int ShutdownPipe[2]{-1, -1};
		std::map<int, std::filesystem::path> WatchMap; // watch descriptor -> absolute directory path
#endif
	};
} // namespace JSL::Async::Watcher
