#pragma once
#include <vector>
#include <filesystem>
#include <regex>
#include <string>
namespace JSL::IO
{
    /*!
     * @brief An object-oriented interface for the locale-independent std::filesystem library, allowing querying and searching of a specified directory and its contents.
     */
    class Directory
    {
        public:
            /*!
             * @brief Construct a Directory object 
             * @param target The path to the directory; either an absolute path or relative to the running location of the code
             * @param recursive If true, any directories found within the target are fully constructed, if false only their name is initialised and they appear as empty directories
             * @param throws filesystem_error if target is not a valid directory 
             */
            Directory(const std::filesystem::path & target, bool recursive = false, size_t maxDepth = -1);

            //! @brief The filepath of the directory
            std::filesystem::path Path;

             //! @brief A list of paths for which ``std::filesystem::is_directory()`` returned true. If the parent was recursive, these are fully populated as Directory objects; otherwise they hold only the directory names
            std::vector<Directory> Directories;

            //! @brief A list of files for which ``std::filesystem::is_regular_file`` returned true.
            std::vector<std::filesystem::path> Files; 

            //! @brief A list of objects which are neither directories or regular files 
            //! @details These will usually be symlinks, socketfiles etc; and can be quieried with ``std::filesystem::file_type``. They are kept separate on the assumption that most users don't want them muddying the water. 
            std::vector<std::filesystem::path> Other; 


            std::vector<std::filesystem::path> ListFiles(bool includeOthers = false);
            std::vector<std::filesystem::path> ListAll(bool includeOthers=false);
            std::vector<std::filesystem::path> ListDirs();

             // /**
            //  * Filters a structure for files matching a glob pattern (e.g., "*.cpp")
            //  * \param pattern The glob pattern (supports * and ?)
            //  */
            std::vector<std::filesystem::path> MatchFiles(std::string matchPattern);
            std::vector<std::filesystem::path> MatchFiles(std::regex matchPattern);
            
            //! @brief A wrapper around Directory(target,recursive).ListFiles()
            static std::vector<std::filesystem::path> list(std::filesystem::path target, bool recursive = false); 
            
            static std::vector<std::filesystem::path> match(std::filesystem::path target, std::string matchPattern, bool recursive = false);
        protected:
            /*!
             * @brief A private constructor for creating 'blank' directories when traversing non-recursively
                @param nameOnly a meaningless parameter with the only purpose being to disambiguate this overload from Director(target), which performs a scan of interior files
             */
            Directory(bool nameOnly, const std::filesystem::path  & target) ;
            Directory(const std::filesystem::path & target, bool recursive, size_t maxDepth, size_t currentDepth);
            bool IsRecursive;

    };

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

    report mkdir(const std::filesystem::path directory,Policy policy = DefaultPolicy);

    report remove(const std::filesystem::path pathToFile, Policy polict = DefaultPolicy);

    /*
        An explicit function that forces a user to acknowledge they're deleting a whole directory and all its contents
    */
    report removeDirectory(const std::filesystem::path pathToDirectory,Policy policy = DefaultPolicy);


}