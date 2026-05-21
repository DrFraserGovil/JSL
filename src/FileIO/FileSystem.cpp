#include <JSL/FileIO/FileSystem.h>
#include <JSL/Vectors/Join.h>
#include <JSL/Display/Log.h>
#include <regex>
#include <algorithm>
#include <filesystem>

namespace JSL::IO
{
	namespace fs = std::filesystem;
	
	Directory::Directory(const fs::path & target,bool recursive,size_t maxDepth) : Directory(target, recursive, maxDepth,0)
	{
		
	}

	Directory::Directory(bool nameOnly, const fs::path & target) : Path(target)
	{
		IsRecursive = false;
	}

	Directory::Directory(const fs::path & target,bool recursive,size_t maxDepth, size_t depth) : Path(target) 
	{
		IsRecursive = recursive && (depth < maxDepth);
		std::error_code ec;
		std::filesystem::directory_iterator it(target, ec);
		for (const auto &element : it)
		{
			auto entry = element.path();
			if(fs::is_regular_file(entry))
			{
				Files.push_back(entry);
				continue;
			}
			if (fs::is_directory(entry))
			{
				if (recursive)
				{
					//triggers the default constructor to continue the recursion
					Directories.push_back(Directory(entry,recursive,maxDepth,depth+1));
				}
				else
				{
					//triggers the 'name only constructor 
					Directories.push_back(Directory(true,entry));
				}
				continue;
			}

			Other.push_back(entry);
		}
	}
	
	
	
	std::vector<std::filesystem::path> Directory::ListFiles(bool includeOthers)
	{
		auto out = Files;
		if (includeOthers){JSL::Vector::append(out,Other);};

		if (IsRecursive)
		{
			for (auto & dir : Directories)
			{
				JSL::Vector::append(out,dir.ListFiles());
			}
		}
		return out;
	}
	std::vector<std::filesystem::path> Directory::ListDirs()
	{
		std::vector<fs::path> out;
		for (auto & dir : Directories)
		{   
			out.push_back(dir.Path);

			if (IsRecursive)
			{       
				JSL::Vector::append(out,dir.ListDirs());    
			}
		}
		return out;
	}
	std::vector<std::filesystem::path> Directory::ListAll(bool includeOthers)
	{
		return JSL::Vector::concat(ListFiles(includeOthers),ListDirs());
	}

	std::vector<std::filesystem::path> Directory::MatchFiles(std::string regexString)
	{

		// Escape special regex characters: . + ^ $ | ( ) [ ] { }
		// We use R"(\$0)" to mean "Put a backslash in front of whatever you matched"
		static const std::regex esc(R"([\.\+\^\$\|\(\)\[\]\{\}\*\?])");
		regexString = std::regex_replace(regexString, esc, R"(\$&)");

		// Convert our escaped \* back into a regex wildcard .*
		// Note: We match the literal backslash and asterisk we just created
		regexString = std::regex_replace(regexString, std::regex(R"(\\\*)"), ".*");

		// Convert our escaped \? back into a regex wildcard .
		regexString = std::regex_replace(regexString, std::regex(R"(\\\?)"), ".");


		std::regex filter(regexString,std::regex_constants::icase); // Case-insensitive often nicer for CLI

		return MatchFiles(filter);
	}
	std::vector<std::filesystem::path> Directory::MatchFiles(std::regex regexFilter)
	{
		std::vector<fs::path> out;
		for (auto file : Files)
		{
		  
			bool matched = std::regex_match(file.filename().string(),regexFilter);
			if (matched){
				out.push_back(file);
			};
		}

		for (auto & dir : Directories)
		{
			JSL::Vector::append(out,dir.MatchFiles(regexFilter));
		}
		return out;
	}

	std::vector<std::filesystem::path> Directory::list(std::filesystem::path target, bool recursive)
	{
		auto top = Directory(target,recursive);
		return top.ListAll(false);
	}
	std::vector<std::filesystem::path> Directory::match(std::filesystem::path target, std::string matchPattern, bool recursive)
	{
		auto top = Directory(target,recursive);
		return top.MatchFiles(matchPattern);
	}


	Policy DefaultPolicy = Strict;




	report mkdir(const std::filesystem::path directory,Policy policy)
	{
		if (fs::exists(directory))
		{
			if (policy == Strict)
			{
				return {false,"Cannot create " + directory.string() + ": already exists"};
			}
			else
			{
				return {true,""}; //a quiet policy treats an extant directory as a full success
			}
		}

		std::error_code ec;
		std::filesystem::create_directories(directory,ec);
		if (ec)
		{
			return {false, "ERROR: " + ec.message()};
		}

		return {true,""};

	}

	report internalErase(const std::filesystem::path path,Policy policy)
	{
		if (!fs::exists(path))
		{
			if (policy == Strict)
			{
				return {false, "Cannot remove " + path.string() + ": does not exist"};
			}
			else
			{
				return {true,""};
			}
		}
		std::error_code ec;
		std::filesystem::remove_all(path,ec);
		if (ec)
		{
			return {false, "ERROR: " + ec.message()};
		}

		return {true,""};
	}

	report remove(const std::filesystem::path pathToFile,Policy policy)
	{
		if (fs::is_directory(pathToFile))
		{
			return {false,"Deleting directory " + pathToFile.string() + " always requires an explicit call to removeDirectory"};
		}
		return internalErase(pathToFile,policy); //removeDirectory is just an aggressive delete, so we can duplicate here
	}

	report removeDirectory(const std::filesystem::path pathToDirectory,Policy policy)
	{
		if(!fs::is_directory(pathToDirectory) && policy == Strict)
		{
			return {false,pathToDirectory.string() + " is not a directory; the Strict policy forbids removeDirectory from removing it."};
		}
		return internalErase(pathToDirectory,policy);
	}
}