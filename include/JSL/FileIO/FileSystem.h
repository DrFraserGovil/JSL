#pragma once
#include <vector>
#include <filesystem>
#include <regex>
#include <string>
namespace JSL::Filesystem
{
    namespace fs = std::filesystem;
    class Structure
    {
        public:
            fs::path Path;
            std::vector<Structure> Directories;
            std::vector<fs::path> Files; 
            std::vector<fs::path> Other; 
            std::vector<fs::path> ListFiles(bool includeOthers = false);
            std::vector<fs::path> ListAll(bool includeOthers=false);
            std::vector<fs::path> ListDirs();

             // /**
            //  * Filters a structure for files matching a glob pattern (e.g., "*.cpp")
            //  * \param pattern The glob pattern (supports * and ?)
            //  */
            std::vector<fs::path> MatchFiles(std::string matchPattern);
            std::vector<fs::path> MatchFiles(std::regex matchPattern);
            
            Structure(fs::path & target, bool recursive = false,int depth=0);

            
        private:
            bool IsRecursive;
    };

    std::vector<fs::path> list(fs::path & target, bool recursive = false);
    
    std::vector<fs::path> match(fs::path & target, std::string matchPattern, bool recursive = false);

    struct report
    {
        bool Successful;
        std::string Message;
    };


    enum Policy{
        Strict,
        Quiet
    };

    extern Policy DefaultPolicy;

    report mkdir(const fs::path& directory,Policy policy = DefaultPolicy);

    report remove(const fs::path & pathToFile, Policy polict = DefaultPolicy);

    /*
        An explicit function that forces a user to acknowledge they're deleting a whole directory and all its contents
    */
    report removeDirectory(const fs::path & pathToDirectory,Policy policy = DefaultPolicy);


    // struct DirectoryContents
    // {
    //     std::vector<
    //     fs::path Path;
    //     PathStatus Status;
    // };

    // /**
    //  * Lists the contents of a directory.
    //  * \param target The directory to scan.
    //  * \param recursive If true, performs a deep scan of all subdirectories.
    //  * \returns A vector of DirectoryEntry objects.
    //  */
    // std::vector<DirectoryEntry> inline listFiles(const fs::path& target, bool recursive = false)
    // {
    //     std::vector<DirectoryEntry> contents;
    //     std::error_code ec;

    //     if (!JSL::pathStatus(target).isDirectory())
    //     {
    //         return contents; 
    //     }

    //     auto scope = [&](auto& it) 
    //     {
    //         for (const auto& entry : it)
    //         {
    //             contents.push_back({
    //                 entry.path(),
    //                 JSL::pathStatus(entry.path())
    //             });
    //         }
    //     };

    //     if (recursive)
    //     {
    //         fs::recursive_directory_iterator it(target, ec);
    //         scope(it);
    //     }
    //     else
    //     {
    //         fs::directory_iterator it(target, ec);
    //         scope(it);
    //     }

    //     return contents;
    // }

    // /**
    //  * Filters a directory for files matching a glob pattern (e.g., "*.cpp")
    //  * \param target The directory to search
    //  * \param pattern The glob pattern (supports * and ?)
    //  * \param recursive Whether to search subfolders
    //  */
    // std::vector<DirectoryEntry> inline listFiles(const fs::path& target, 
    //                                         std::string pattern, 
    //                                         bool recursive = false)
    // {
    //     std::vector<DirectoryEntry> matches;
        
    //     // Convert Glob pattern to Regex pattern
    //     // Escape regex special chars, then convert * to .* and ? to .
    //     // Simple implementation for common use cases:
    //     std::string regexString = pattern;

    //     // Escape special regex characters: . + ^ $ | ( ) [ ] { }
    //     // We use R"(\$0)" to mean "Put a backslash in front of whatever you matched"
    //     static const std::regex esc(R"([\.\+\^\$\|\(\)\[\]\{\}\*\?])");
    //     regexString = std::regex_replace(regexString, esc, R"(\$&)");

    //     // Convert our escaped \* back into a regex wildcard .*
    //     // Note: We match the literal backslash and asterisk we just created
    //     regexString = std::regex_replace(regexString, std::regex(R"(\\\*)"), ".*");

    //     // 4. Convert our escaped \? back into a regex wildcard .
    //     regexString = std::regex_replace(regexString, std::regex(R"(\\\?)"), ".");

    //     std::regex filter(regexString,std::regex_constants::icase); // Case-insensitive often nicer for CLI

    //     auto allEntries = JSL::listFiles(target, recursive);        
    //     std::copy_if(allEntries.begin(), allEntries.end(), std::back_inserter(matches),
    //         [&](const DirectoryEntry& entry) {
    //             // Match against the filename only, not the full path
    //             return std::regex_match(entry.Path.filename().string(), filter);
    //         });
    //     return matches;
    // }
}