#pragma once
#include <filesystem>
#include <string>
#include "pathStatus.h" 

namespace JSL
{
    struct mkdirReturn
    {
        bool Successful;
        std::string Message;
    };

    /*!
     * Portably creates a directory and any missing parent directories.
     * \param directory Path to the desired directory
     * \return A mkdirReturn object
     */
    mkdirReturn inline mkdir(const std::filesystem::path& directory)
    {
        mkdirReturn output{ true, "" };
        
        // Check existing state
        auto status = JSL::pathStatus(directory);
        
        if (status.exists)
        {
            if (status.isDirectory())
            {
                output.Message = "\tDirectory '" + directory.string() + "' already exists. No action taken.\n";
                //output already set to successful
            }
            else
            {
                output.Message = "ERROR: '" + directory.string() + "' exists but is not a directory. Cannot overwrite.\n";
                output.Successful = false;
            }
            return output;
        }
        
        // Attempt (recursive) creation
        std::error_code ec;
        if (std::filesystem::create_directories(directory, ec))
        {
            output.Message = "Directory '" + directory.string() + "' created successfully.\n";
        }
        else
        {
            // If it returns false and no error code, it means it already existed 
             // (but we checked that above, so this handlesother issues).
            if (ec)
            {
                output.Successful = false;
                output.Message = "ERROR: Failed to create '" + directory.string()+ "'. System says: " + ec.message() + "\n";
            }
        }

        return output;
    }
}