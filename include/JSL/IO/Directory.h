#pragma once
#include <filesystem>
#include <regex>
#include <string>
#include <set>
#include <optional>
#include "glob.h"
namespace JSL::IO
{
	/*!
	 * @brief An object-oriented interface for the locale-independent std::filesystem library, allowing querying and searching of a specified directory and its contents.
	 */
	class Directory
	{
	public:
		/*! @brief Construct a Directory object representing a snapshot of the current contents of a directory
		 * @param target The path to the directory; either an absolute path or relative to the running location of the code
		 * @param recursive If true, any directories found within the target are fully constructed, if false only their name is initialised and they appear as empty directories
		 * @param throws filesystem_error if target is not a valid directory
		 */
		static Directory Snapshot(const std::filesystem::path &target, bool recursive = false);

		/*! @brief Constructs a Directory object representing a snapshot of a single directory and all its contents, automatically recursing up to a specified limit
		 * @param target The path to the directory; either an absolute path or relative to the running location of the code
		 * @param maxDepth The maximum number of sub-directories to visit (0 is equivalent to no recursion)
		 * @param throws filesystem_error if target is not a valid directory
		 */
		static Directory Snapshot(const std::filesystem::path &target, size_t maxDepth);

		/*! @brief Constructs a Directory object representing a snapshot of a single directory and all its contents, automatically recursing up to a specified limit, and ignoring specified files
		 * @param excludePattern a regular expression; matches are excluded from both the file list and further recursion
		 * @param maxDepth The maximum number of sub-directories to visit. maxDepth = 0 is no recursion
		 */
		static Directory Snapshot(const std::filesystem::path &target, std::regex ExcludePattern, size_t maxDepth = -1);
		
		/*! @brief Constructs a Directory object representing a snapshot of a single directory and all its contents, automatically recursing up to a specified limit, and ignoring specified files
		 * @param excludePattern a glob-expression for files and directories to ignore 
		 * @param maxDepth The maximum number of sub-directories to visit. maxDepth = 0 is no recursion
		 */
		template<class T>
		requires std::convertible_to<T,std::string>
		static Directory Snapshot(const std::filesystem::path &target, T & ExcludePattern, size_t maxDepth = -1)
		{
			return Snapshot(target, globToRegex(ExcludePattern),maxDepth);
		}

		/*!
		 * @brief Resets the directory and re-initialises it with the new recursion directive
		 * @param recursive If true, any directories found within the target are fully constructed, if false only their name is initialised and they appear as empty directories. This overrides the previous recursion parameter.
		 */
		void Rescan(bool recursive = false);
		/*!
		 * @brief Resets the directory and re-initialises it with the new recursion directive
		 * @param maxDepth The maximum number of sub-directories to visit. This overrides the previous recursion parameter.
		 */
		void Rescan(size_t maxDepth);
		/*!
		 * @brief Resets the directory and re-initialises it with the new recursion directive
		 * @param excludePattern a regular expression; matches are excluded from both the file list and further recursion
		 * @param maxDepth The maximum number of sub-directories to visit. This overrides the previous recursion parameter.
		 */
		void Rescan(std::regex excludePattern, size_t maxDepth = -1);

		/*!
		 * @brief Resets the directory and re-initialises it with the new recursion directive
		 * @param excludePattern a glob-expression for files and directories to ignore 
		 * @param maxDepth The maximum number of sub-directories to visit. This overrides the previous recursion parameter.
		 */
		template<class T>
		requires std::convertible_to<T,std::string>
		void Rescan(T & excludePattern, size_t maxDepth = -1)
		{
			Rescan(globToRegex(excludePattern),maxDepth);
		}

		//! @brief The filepath of the directory
		std::filesystem::path Path;

		//! @brief A list of paths for which ``std::filesystem::is_directory()`` returned true. If the parent was recursive, these are fully populated as Directory objects; otherwise they hold only the directory names
		std::set<Directory> Directories;

		//! @brief A list of files for which ``std::filesystem::is_regular_file`` returned true.
		std::set<std::filesystem::path> Files;

		//! @brief A list of objects which are neither directories or regular files
		//! @details These will usually be symlinks, socketfiles etc; and can be quieried with ``std::filesystem::file_type``. They are kept separate on the assumption that most users don't want them muddying the water.
		std::set<std::filesystem::path> Others;

		/*!
		 * @brief Get a list of the file-object within the current snapshot
		 * @param useRecursion If true (and the object was initialised recursively), it lists all files within the tree, if false, only outputs files within the top level.
		 * @param includeOthers If true, include the Others in the file list 
		 * @return A set of filepaths of all files present at the time of the last scan 
		 */
		std::set<std::filesystem::path> ListFiles(bool useRecursion = true,bool includeOthers = false) const;
		/*!
		 * @brief Get a list of *all* filesystem objects within the current snapshot
		 * @param useRecursion If true (and the object was initialised recursively), it lists all files within the tree, if false, only outputs files within the top level.
		 * @return A set of filepaths of all files present at the time of the last scan 
		 */
		std::set<std::filesystem::path> ListAll(bool useRecursion = true) const;

		/*!
		 * @brief Get a list of all directories within the current snapshot
		 * @param useRecursion If true (and the object was initialised recursively), it lists all directories within the tree, if false, only outputs directories within the top level.
		 * @return A set of filepaths of all directories present at the time of the last scan 
		 * @note The Path of the calling object is *not* included in this output
		 */
		std::set<std::filesystem::path> ListDirs(bool useRecursion = true) const;

		/*! @brief Get a list of files matching the specified regex pattern
			@param matchPattern A regular expression used to identify files to include
			@returns A subset of the output of ListFiles, matching the regular expression
		*/ 
		std::set<std::filesystem::path> MatchFiles(std::regex matchPattern) const;
		
		/*! @brief Get a list of files matching the specified regex pattern
			@param matchPattern A glob pattern used to identify files to include
			@returns A subset of the output of ListFiles, matching the glob
		*/ 
		template<class T>
		requires std::convertible_to<T,std::string>
		std::set<std::filesystem::path> MatchFiles(T matchPattern) const
		{
			return MatchFiles(globToRegex(matchPattern));
		}
		//! @brief A wrapper around ``Directory::Snapshot(target,recursive).ListFiles()``
		static std::set<std::filesystem::path> list(std::filesystem::path target, bool recursive = false);


		//! @brief A wrapper around ``Directory::Snapshot(target,recursive).MatchFiles(matchPattern)``
		//! @details This is not a template (like the others) because there's no risk of a char* primitive silently decaying into a boolean, so we can just force a direct string-cast
		static std::set<std::filesystem::path> match(std::filesystem::path target, std::string matchPattern, bool recursive = false);

		//! @brief Spaceship operator which defines the ordering of Directory objects via their internal path. 
		//! @details A valid path is guaranteed to point to a single unique file , so this is suitable for unique identification. Clashes *are* possible due to either changes in the filesystem between snapshots, or due to misuse of relative paths. If this is a concern, use ``fs::make_canonical`` when constructing the directory.
		auto operator<=>(const Directory & test) const
		{
			return Path <=> test.Path;
		}
	private:
		/*!
		 * @brief A private constructor for creating 'blank' directories to act as leaves when traversing non-recursively
		 * @param target The path to the directory
		 */
		Directory(const std::filesystem::path &target);

		/*!
		 * @brief Internal constructor for the Directory class
		 * @param target The path to the directory; either an absolute path or relative to the running location of the code
		 * @param recursive If true, any directories found within the target are fully constructed, if false only their name is initialised and they appear as empty directories
		 * @param maxDepth the value of currentDepth at which recursion is forced to terminate
		 * @param currentDepth a tracker for the recursion; incremented by 1 for every nested subdirectory
		 * @param throws filesystem_error if target is not a valid directory
		 */
		Directory(const std::filesystem::path &target, size_t maxDepth, size_t currentDepth,std::optional<std::regex> excludePattern = std::nullopt);

		//! Flag to indicate if a directory was recursively scanned 
		bool IsRecursive = false;

		/*! @brief Executes the filesystem scans required by constructors and Rescan calls 
			@param depth The current recursion level 
			@param maxDepth the limit at which recursion stops
			@param excluder The regex filter to omit things. Optional so that if unused we get a performance boost
		*/
		void InternalScan(size_t depth, size_t maxDepth,std::optional<std::regex> excluder);

		//! @brief Clears the internal storage of the object
		void Reset();
	};
}