#include <JSL/FileIO/FileSystem.h>
#include <JSL/Vectors/Join.h>
#include <regex>
#include <algorithm>
#include <filesystem>
namespace JSL::Filesystem
{
    namespace fs = std::filesystem;
    Structure::Structure(fs::path target, bool recursive,int depth) : Path(target), IsRecursive(recursive)
    {
        std::error_code ec;
        auto walk = [&](auto& it) 
        {
           
                for (const auto& element : it)
                {
                    auto entry = element.path();
                    if(fs::is_regular_file(entry))
                    {
                        Files.push_back(entry);
                        continue;
                    }
                    if (fs::is_directory(entry))
                    {
                        Directories.emplace_back(entry,recursive,depth+1);
                        continue;
                    }

                    Other.push_back(entry);
                }
        };

        if (recursive)
        {
            std::filesystem::recursive_directory_iterator it(target, ec);
            walk(it);
        }
        else
        {
            if (depth == 0)
            {
                std::filesystem::directory_iterator it(target, ec);
                walk(it);
            }
        }
    }

    std::vector<std::filesystem::path> Structure::ListFiles(bool includeOthers)
    {
        auto out = Files;
        if (includeOthers){JSL::append(out,Other);};

        if (IsRecursive)
        {
            for (auto & dir : Directories)
            {
                JSL::append(out,dir.ListFiles());
            }
        }
        return out;
    }
    std::vector<std::filesystem::path> Structure::ListDirs()
    {
        std::vector<fs::path> out;
        for (auto & dir : Directories)
        {   
            out.push_back(dir.Path);
            if (IsRecursive)
            {       
                JSL::append(out,dir.ListDirs());    
            }
        }
        return out;
    }
    std::vector<std::filesystem::path> Structure::ListAll(bool includeOthers)
    {
        return JSL::concat(ListFiles(includeOthers),ListDirs());
    }

    std::vector<std::filesystem::path> Structure::MatchFiles(std::string regexString)
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
    std::vector<std::filesystem::path> Structure::MatchFiles(std::regex regexFilter)
    {
        std::vector<fs::path> out;
        for (auto file : Files)
        {
            bool matched = std::regex_match(file.filename().string(),regexFilter);
            if (matched){out.push_back(file);};
        }

        for (auto & dir : Directories)
        {
            JSL::append(out,dir.MatchFiles(regexFilter));
        }
        return out;
    }

    std::vector<std::filesystem::path> list(std::filesystem::path target, bool recursive)
    {
        auto top = Structure(target,recursive);
        return top.ListAll();
    }
    std::vector<std::filesystem::path> match(std::filesystem::path target, std::string matchPattern, bool recursive)
    {
        auto top = Structure(target,recursive);
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