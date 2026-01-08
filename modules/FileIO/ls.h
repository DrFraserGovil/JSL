#pragma once
#include <vector>
#include <filesystem>
#include "pathStatus.h"
#include <regex>
#include <algorithm>
namespace JSL
{
    struct DirectoryEntry
    {
        std::filesystem::path Path;
        PathStatus Status;
    };

    /**
     * Lists the contents of a directory.
     * \param target The directory to scan.
     * \param recursive If true, performs a deep scan of all subdirectories.
     * \returns A vector of DirectoryEntry objects.
     */
    std::vector<DirectoryEntry> inline listFiles(const std::filesystem::path& target, bool recursive = false)
    {
        std::vector<DirectoryEntry> contents;
        std::error_code ec;

        if (!JSL::pathStatus(target).isDirectory())
        {
            return contents; 
        }

        auto scope = [&](auto& it) 
        {
            for (const auto& entry : it)
            {
                contents.push_back({
                    entry.path(),
                    JSL::pathStatus(entry.path())
                });
            }
        };

        if (recursive)
        {
            std::filesystem::recursive_directory_iterator it(target, ec);
            scope(it);
        }
        else
        {
            std::filesystem::directory_iterator it(target, ec);
            scope(it);
        }

        return contents;
    }

    /**
     * Filters a directory for files matching a glob pattern (e.g., "*.cpp")
     * \param target The directory to search
     * \param pattern The glob pattern (supports * and ?)
     * \param recursive Whether to search subfolders
     */
    std::vector<DirectoryEntry> inline listFiles(const std::filesystem::path& target, 
                                            std::string pattern, 
                                            bool recursive = false)
    {
        std::vector<DirectoryEntry> matches;
        
        // Convert Glob pattern to Regex pattern
        // Escape regex special chars, then convert * to .* and ? to .
        // Simple implementation for common use cases:
        std::string regexString = pattern;

        // Escape special regex characters: . + ^ $ | ( ) [ ] { }
        // We use R"(\$0)" to mean "Put a backslash in front of whatever you matched"
        static const std::regex esc(R"([\.\+\^\$\|\(\)\[\]\{\}\*\?])");
        regexString = std::regex_replace(regexString, esc, R"(\$&)");

        // Convert our escaped \* back into a regex wildcard .*
        // Note: We match the literal backslash and asterisk we just created
        regexString = std::regex_replace(regexString, std::regex(R"(\\\*)"), ".*");

        // 4. Convert our escaped \? back into a regex wildcard .
        regexString = std::regex_replace(regexString, std::regex(R"(\\\?)"), ".");

        std::regex filter(regexString,std::regex_constants::icase); // Case-insensitive often nicer for CLI

        auto allEntries = JSL::listFiles(target, recursive);        
        std::copy_if(allEntries.begin(), allEntries.end(), std::back_inserter(matches),
            [&](const DirectoryEntry& entry) {
                // Match against the filename only, not the full path
                return std::regex_match(entry.Path.filename().string(), filter);
            });
        return matches;
    }
}