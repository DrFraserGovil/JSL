#include <JSL/IO/Directory.h>
#include <JSL/Log.h>
#include <JSL/internal/error.h>
#include <filesystem>
#include <regex>
namespace JSL::IO
{
	namespace fs = std::filesystem;

	Directory Directory::Snapshot(const fs::path &target, bool recursive)
	{
		size_t maxDepth = recursive ? -1 : 0;
		return Directory(target, maxDepth, 0);
	}
	Directory Directory::Snapshot(const fs::path &target, size_t maxDepth)
	{
		return Directory(target, maxDepth, 0);
	}

	Directory Directory::Snapshot(const fs::path &target, std::regex excludePattern, size_t maxDepth)
	{
		return Directory(target, maxDepth, 0, excludePattern);
	}

	Directory::Directory(const fs::path &target) : Path(target)
	{
		IsRecursive = false;
	}

	Directory::Directory(const fs::path &target, size_t maxDepth, size_t depth, std::optional<std::regex> excludePattern) : Path(target)
	{
		InternalScan(depth, maxDepth, excludePattern);
	}

	void Directory::Reset()
	{
		Directories.clear();
		Files.clear();
		Others.clear();
	}

	void Directory::Rescan(bool recursive)
	{
		Reset();
		size_t limit = (recursive ? -1 : 0);
		InternalScan(0, limit, std::nullopt);
	}
	void Directory::Rescan(size_t maxDepth)
	{
		Reset();
		InternalScan(0, maxDepth, std::nullopt);
	}
	void Directory::Rescan(std::regex excludePattern, size_t maxDepth)
	{
		Reset();
		InternalScan(0, maxDepth, excludePattern);
	}

	void Directory::InternalScan(size_t depth, size_t maxDepth, std::optional<std::regex> excluder)
	{
		if (!fs::exists(Path))
		{
			JSL::internal::LibraryError("Bad Directory", JSL_LOCATION) << Path.string() << " does not exist at the specified location (" << fs::current_path() << ")";
		}
		if (!fs::is_directory(Path))
		{
			JSL::internal::LibraryError("Bad Directory", JSL_LOCATION) << Path.string() << " is not a directort at the specified location (" << fs::canonical(Path) << ")";
		}
		IsRecursive = (depth < maxDepth);
		std::error_code ec;
		std::error_code ec2;
		std::filesystem::directory_iterator it(Path, ec);
		for (const auto &element : it)
		{
			auto entry = element.path();
			auto relpath = fs::relative(element, Path, ec).generic_string(); // we use the relpath for
			if (ec) continue;
			bool blacklisted = false;
			if (excluder)
			{
				std::filesystem::path root = Path;
				for (size_t i = 0; i < depth; ++i) root = root.parent_path();
				auto relativeEntry = entry.lexically_relative(root).generic_string();
				auto bareName = entry.filename().generic_string();
				blacklisted = std::regex_match(relativeEntry, excluder.value()) ||
							  std::regex_match(bareName, excluder.value());
			}
			if (!blacklisted)
			{
				auto status = element.symlink_status(ec);
				if (ec) continue;

				if (fs::is_regular_file(status))
				{

					auto size = element.file_size(ec);
					auto mtime = element.last_write_time(ec2);
					if (ec || ec2)
					{
						Others.insert(entry);
						continue;
					}
					MetaData.insert({entry, mtime, size});
					Files.insert(entry);
					continue;
				}
				else if (fs::is_directory(status))
				{
					if (IsRecursive)
					{
						// triggers the default constructor to continue the recursion
						Directories.insert(Directory(entry, maxDepth, depth + 1, excluder));
					}
					else
					{
						// triggers the 'name only constructor
						Directories.insert(Directory(entry));
					}
					continue;
				}
				else
				{
					Others.insert(entry);
				}
			}
		}
	}

	std::set<std::filesystem::path> Directory::ListFiles(bool listAllFiles, bool includeOthers) const
	{
		auto out = Files;
		if (includeOthers) { out.insert(Others.begin(), Others.end()); };

		if (listAllFiles && IsRecursive)
		{
			for (auto &dir : Directories)
			{
				out.merge(dir.ListFiles(listAllFiles, includeOthers));
			}
		}
		return out;
	}
	std::set<FileMetadata> Directory::ListMetadata(bool useRecursion) const
	{
		auto out = MetaData;

		if (IsRecursive && useRecursion)
		{
			for (auto &dir : Directories)
			{
				out.merge(dir.ListMetadata(useRecursion));
			}
		}
		return out;
	}

	std::set<std::filesystem::path> Directory::ListDirs(bool useRecursion) const
	{
		std::set<fs::path> out;
		for (auto &dir : Directories)
		{
			out.insert(dir.Path);
			if (IsRecursive && useRecursion)
			{
				auto tmp = dir.ListDirs(useRecursion);
				out.merge(tmp);
			}
			// else
			// {
			// 	out.insert(dir.Path); //else just tag in their name
			// }
		}
		return out;
	}
	std::set<std::filesystem::path> Directory::ListAll(bool useRecursion) const
	{
		auto out = ListFiles(useRecursion, true);
		out.merge(ListDirs(useRecursion));
		return out;
	}
	std::set<std::filesystem::path> Directory::ListOthers(bool useRecursion) const
	{

		auto out = Others;

		if (IsRecursive && useRecursion)
		{
			for (auto &dir : Directories)
			{
				out.merge(dir.ListOthers(useRecursion));
			}
		}
		return out;
	}

	std::set<std::filesystem::path> Directory::MatchFiles(std::regex regexFilter) const
	{
		std::set<fs::path> out;
		for (auto file : Files)
		{

			bool matched = std::regex_match(file.filename().string(), regexFilter);
			if (matched)
			{
				out.insert(file);
			};
		}

		for (auto &dir : Directories)
		{
			out.merge(dir.MatchFiles(regexFilter));
		}
		return out;
	}

	std::set<std::filesystem::path> Directory::list(std::filesystem::path target, bool recursive)
	{
		auto top = Snapshot(target, recursive);
		return top.ListAll();
	}
	std::set<std::filesystem::path> Directory::match(std::filesystem::path target, std::string matchPattern, bool recursive)
	{
		auto top = Snapshot(target, recursive);
		return top.MatchFiles(matchPattern);
	}
} // namespace JSL::IO
