#include <JSL/Async/Watcher/FileWatcher.h>
#include <JSL/IO/Glob.h>
#include <JSL/Log.h>
#include <JSL/internal/error.h>
namespace JSL::Async::Watcher
{
	File::File()
	{
		Initialised = false;
	}

	File::File(std::filesystem::path path, bool recursive, callback fcn)
	{
		Initialise(path, recursive, fcn);
	}

	void File::Initialise(std::filesystem::path path, bool recursive, callback fcn)
	{
		if (Running.load(std::memory_order_acquire))
		{
			JSL::internal::LibraryError("Invalid state", JSL_LOCATION) << "Cannot re-initialise a File watcher while it is running";
		}
		RootPath = std::move(path);
		IsRecursive = recursive;
		Callback = std::move(fcn);
		Initialised = true;
	}

	bool File::IsWhitelisted(const std::filesystem::path &relativePath) const
	{
		if (Whitelist.empty()) return true;

		auto rootRelative = relativePath.lexically_relative(RootPath);
		std::string relStr = rootRelative.generic_string();
		std::string bareName = relativePath.filename().generic_string();

		return (std::regex_match(relStr, combinedWhitelist) || std::regex_match(bareName, combinedWhitelist));
	}
	bool File::IsBlacklisted(const std::filesystem::path &relativePath) const
	{
		if (Blacklist.empty()) return false;
		auto rootRelative = relativePath.lexically_relative(RootPath);
		std::string relStr = rootRelative.generic_string();
		std::string bareName = relativePath.filename().generic_string();

		return (std::regex_match(relStr, combinedBlacklist) || std::regex_match(bareName, combinedBlacklist));
	}

	void File::AddWhiteList(std::string whiteGlob)
	{
		Whitelist.push_back(whiteGlob);
		combinedWhitelist = JSL::IO::multiGlobToRegex(Whitelist);
	}

	void File::AddBlackList(std::string blackGlob)
	{
		Blacklist.push_back(blackGlob);
		combinedBlacklist = JSL::IO::multiGlobToRegex(Blacklist);
	}

	JSL::IO::Directory File::TakeSnapshot()
	{
		size_t depth = IsRecursive ? static_cast<size_t>(-1) : 0;
		try
		{
			return JSL::IO::Directory::Snapshot(RootPath, Blacklist, depth);
		}
		catch (...)
		{
			LOG(WARN) << "A top-level watched directory was just deleted or moved: this watcher is now disabled";
			CriticalErrorState = true;
			return JSL::IO::Directory::Empty();
		}
	}
	void File::InitialSnapshot()
	{
		auto snap = TakeSnapshot();
		if (CriticalErrorState)
		{
			JSL::internal::LibraryError("Bad filewatch", JSL_LOCATION) << "Could not perform the initial directory snapshot for the watcher";
		}
		PreviousMeta = snap.ListMetadata();
		PreviousDirs = snap.ListDirs();
		PreviousOthers = snap.ListOthers();
	}

	std::set<FileChange> File::ComputeDiff()
	{
		auto newSnapshot = TakeSnapshot();

		std::set<FileChange> output;
		ComputeDiff_Meta(output, newSnapshot);
		ComputeDiff_Path(output, newSnapshot);

		return output;
	}
	void File::ComputeDiff_Meta(std::set<FileChange> &output, const JSL::IO::Directory &currentSnapshot)
	{
		auto newMeta = currentSnapshot.ListMetadata();
		auto oldIt = PreviousMeta.begin();
		auto newIt = newMeta.begin();

		// because sets are ordered, we can do a single pass and infer creation/deletion automatically
		// We also don't need to do a blacklist check as the Directory already did it for us
		while (oldIt != PreviousMeta.end() && newIt != newMeta.end())
		{
			// file in old that is missing in new: Deletion
			if (oldIt->Path < newIt->Path)
			{
				if (IsWhitelisted(oldIt->Path)) output.insert({oldIt->Path, ChangeType::Delete, ObjectType::File});
				++oldIt;
			}
			// file in new that is missing in old: Creation
			else if (newIt->Path < oldIt->Path)
			{
				if (IsWhitelisted(newIt->Path)) output.insert({newIt->Path, ChangeType::Create, ObjectType::File});
				++newIt;
			}
			else
			{
				// file present in both, but changes in write time or size: Modification
				if ((oldIt->LastWrite != newIt->LastWrite || oldIt->Size != newIt->Size) && IsWhitelisted(oldIt->Path))
				{
					output.insert({oldIt->Path, ChangeType::Modify, ObjectType::File});
				}
				// else: no change to file, so ignore it
				++oldIt;
				++newIt;
			}
		}
		/// Now cleanup from the 'ends' of the linear sweep

		// if the old iterator has more to go, then they were all deleted
		for (; oldIt != PreviousMeta.end(); ++oldIt)
		{
			if (IsWhitelisted(oldIt->Path)) output.insert({oldIt->Path, ChangeType::Delete, ObjectType::File});
		}
		// if the new iterator has more to go, then they were all created
		for (; newIt != newMeta.end(); ++newIt)
		{
			if (IsWhitelisted(newIt->Path)) output.insert({newIt->Path, ChangeType::Create, ObjectType::File});
		}

		std::swap(newMeta, PreviousMeta);
	}

	void File::ComputeDiff_Path(std::set<FileChange> &output, const JSL::IO::Directory &currentSnapshot)
	{
		auto directories = currentSnapshot.ListDirs();
		auto others = currentSnapshot.ListOthers();

		auto diffPresence = [&](const std::set<std::filesystem::path> &oldSet, const std::set<std::filesystem::path> &newSet, ObjectType object) {
			for (const auto &p : oldSet)
			{
				if (!newSet.count(p) && IsWhitelisted(p)) output.insert({p, ChangeType::Delete, object});
			}
			for (const auto &p : newSet)
			{
				if (!oldSet.count(p) && IsWhitelisted(p)) output.insert({p, ChangeType::Create, object});
			}
		};
		diffPresence(PreviousDirs, directories, ObjectType::Directory);
		std::swap(PreviousDirs, directories);

		diffPresence(PreviousOthers, others, ObjectType::Other);
		std::swap(PreviousOthers, others);
	}

	void File::ProcessBatch()
	{
		auto diff = ComputeDiff();
		if (!diff.empty())
		{
			for (auto &d : diff)
			{
				if (PlatformWatchFiles || d.Object == ObjectType::Directory)
				{
					if (d.Change == ChangeType::Create)
					{
						AddWatch(d.Path);
					}
					else if (d.Change == ChangeType::Delete)
					{
						RemoveWatch(d.Path);
					}
				}
			}
			Callback(diff);
		}
	}
	void File::Start()
	{
		if (!Initialised)
		{
			AbortStartup("File watcher started before Initialise() was called");
		}
		if (Running.exchange(true)) return;

		CreateShutdownSystem();
		InitialisePlatformWatchers();
		InitialSnapshot();
		AddWatch(RootPath, true);
		for (auto &dir : PreviousDirs)
		{
			AddWatch(dir);
		}

		WorkerThread = std::thread(&File::Run, this);
	}
} // namespace JSL::Async::Watcher
