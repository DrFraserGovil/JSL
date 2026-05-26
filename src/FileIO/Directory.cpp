#include <JSL/FileIO/Directory.h>
#include <regex>
#include <algorithm>
#include <filesystem>
#include <JSL/Display/Log.h>
namespace JSL::IO
{
	namespace fs = std::filesystem;
	
	Directory Directory::Snapshot(const fs::path & target,bool recursive)  
	{
		size_t maxDepth = recursive ? -1 : 0;
		return Directory(target,maxDepth,0);	
	}
	Directory Directory::Snapshot(const fs::path & target,size_t maxDepth)  
	{
		return Directory(target,maxDepth,0);	
	}

	Directory Directory::Snapshot(const fs::path & target,std::regex excludePattern, size_t maxDepth)  
	{
		return Directory(target,maxDepth,0,excludePattern);	
	}

	Directory::Directory(const fs::path & target) : Path(target)
	{
		IsRecursive = false;
	}

	Directory::Directory(const fs::path & target,size_t maxDepth, size_t depth,std::optional<std::regex> excludePattern) : Path(target) 
	{
		InternalScan(depth,maxDepth, excludePattern);	
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
		InternalScan(0,limit,std::nullopt);
	}
	void Directory::Rescan(size_t maxDepth)
	{
		Reset();
		InternalScan(0,maxDepth,std::nullopt);
	}
	void Directory::Rescan(std::regex excludePattern, size_t maxDepth)
	{
		Reset();
		InternalScan(0,maxDepth,excludePattern);
	}

	void Directory::InternalScan(size_t depth, size_t maxDepth,std::optional<std::regex> excluder)
	{
		IsRecursive = (depth < maxDepth);
		std::error_code ec;
		std::filesystem::directory_iterator it(Path, ec);
		for (const auto &element : it)
		{
			auto entry = element.path();
			if (!excluder || !std::regex_match(entry.string(),excluder.value()))
			{
				if(fs::is_regular_file(entry))
				{
					Files.insert(entry);
					continue;
				}
				if (fs::is_directory(entry))
				{
					if (IsRecursive)
					{
						//triggers the default constructor to continue the recursion
						Directories.insert(Directory(entry,maxDepth,depth+1,excluder)); 
					}
					else
					{
						//triggers the 'name only constructor 
						Directories.insert(Directory(entry));
					}
					continue;
				}

				Others.insert(entry);
			}
		}
		
	}
	
	std::set<std::filesystem::path> Directory::ListFiles(bool listAllFiles, bool includeOthers) const
	{
		auto out = Files;
		if (includeOthers){out.insert(Others.begin(),Others.end());};

		if (listAllFiles && IsRecursive)
		{
			for (auto & dir : Directories)
			{
				out.merge(dir.ListFiles(listAllFiles,includeOthers));
			}
		}
		return out;
	}
	std::set<std::filesystem::path> Directory::ListDirs(bool useRecursion) const
	{
		std::set<fs::path> out;
		for (auto & dir : Directories)
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
		auto out = ListFiles(useRecursion,true);
		out.merge(ListDirs(useRecursion));
		return out; 
	}


	std::set<std::filesystem::path> Directory::MatchFiles(std::regex regexFilter) const
	{
		std::set<fs::path> out;
		for (auto file : Files)
		{
		  
			bool matched = std::regex_match(file.filename().string(),regexFilter);
			if (matched){
				out.insert(file);
			};
		}

		for (auto & dir : Directories)
		{
			out.merge(dir.MatchFiles(regexFilter));
		}
		return out;
	}

	std::set<std::filesystem::path> Directory::list(std::filesystem::path target, bool recursive)
	{
		auto top = Snapshot(target,recursive);
		return top.ListAll();
	}
	std::set<std::filesystem::path> Directory::match(std::filesystem::path target, std::string matchPattern, bool recursive)
	{
		auto top = Snapshot(target,recursive);
		return top.MatchFiles(matchPattern);
	}
}