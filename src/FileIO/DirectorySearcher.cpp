#include <JSL/FileIO/DirectorySearcher.h>
#include <JSL/Vectors/Join.h>
#include <regex>
#include <algorithm>
#include <filesystem>
namespace JSL::Filesystem
{
    namespace fs = std::filesystem;
    Structure::Structure(fs::path & target, bool recursive,int depth) : Path(target), IsRecursive(recursive)
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

    std::vector<std::filesystem::path> Structure::AllFiles()
    {
        auto out = Files;
        if (IsRecursive)
        {
            for (auto & dir : Directories)
            {
                JSL::append(out,dir.AllFiles());
            }
        }
        return out;
    }

    std::vector<std::filesystem::path> Structure::MatchingFiles(std::string regexString)
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

        return MatchingFiles(filter);
    }
    std::vector<std::filesystem::path> Structure::MatchingFiles(std::regex regexFilter)
    {
        std::vector<fs::path> out;
        for (auto file : Files)
        {
            bool matched = std::regex_match(file.filename().string(),regexFilter);
            if (matched){out.push_back(file);};
        }

        for (auto & dir : Directories)
        {
            JSL::append(out,dir.MatchingFiles(regexFilter));
        }
    }

    std::vector<std::filesystem::path> listFiles(std::filesystem::path & target, bool recursive)
    {
        auto top = Structure(target,recursive);
        return top.AllFiles();
    }
    std::vector<std::filesystem::path> matchFiles(std::filesystem::path & target, std::string matchPattern, bool recursive)
    {
        auto top = Structure(target,recursive);
        return top.MatchingFiles(matchPattern);
    }

}