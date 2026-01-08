#pragma once
#include <filesystem>
#include <string>

namespace fs = std::filesystem;

namespace JSL
{
    enum class PathType 
    { 
        NotFound, 
        File, 
        Directory, 
        Symlink,
        Other 
    };

    struct PathStatus
    {
        bool exists;
        PathType type;

        bool isFile() const { return type == PathType::File; }
        bool isDirectory() const { return type == PathType::Directory; }
    };

    /*!
     * Checks for the existence of the provided file location, and determines its type
     * \param path The path to check.
     * \returns A PathStatus object.
     */
    PathStatus inline pathStatus(const std::string& path)
    {
        std::error_code ec; // Prevents exceptions if the path is invalid
        fs::file_status s = fs::status(path, ec);

        if (ec || !fs::exists(s))
        {
            return { false, PathType::NotFound };
        }

        PathType type = PathType::Other;

        if (fs::is_regular_file(s))      { type = PathType::File; }
        else if (fs::is_directory(s))   { type = PathType::Directory; }
        else if (fs::is_symlink(s))     { type = PathType::Symlink; }

        return { true, type };
    }
}