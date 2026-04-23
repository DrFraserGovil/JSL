#pragma once
#include <filesystem>
#include <string>
#include "pathStatus.h"

namespace JSL
{
    struct rmReturn
    {
        bool Successful;
        std::string Message;
    };

    /**
     * Portably removes a file or directory tree.
     * \param path The path to remove.
     * \param recursive If true, will delete directories and their contents.
     * \return A rmReturn object.
     */
    rmReturn inline rm(const std::filesystem::path& path, bool recursive = false)
    {
        rmReturn output{ true, "" };
        auto status = JSL::pathStatus(path.string());

        if (!status.exists)
        {
            output.Message = "Request to remove '" + path.string() + "' ignored: Path does not exist.\n";
            return output;
        }

        if (status.isDirectory() && !recursive)
        {
            output.Successful = false;
            output.Message = "ERROR: Cannot remove directory '" + path.string() + "' without recursive flag.\n";
            return output;
        }

        std::error_code ec;
        // remove_all handles both files and directories recursively
        std::uintmax_t deletedCount = std::filesystem::remove_all(path, ec);

        if (ec)
        {
            output.Successful = false;
            output.Message = "ERROR: Failed to remove '" + path.string() + "'. System says: " + ec.message() + "\n";
        }
        else
        {
            output.Message = "Successfully removed '" + path.string() + "' (" + std::to_string(deletedCount) + " items).\n";
        }

        return output;
    }
}