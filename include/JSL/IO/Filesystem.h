#pragma once
#include <filesystem>
namespace JSL::IO
{

	/*!
		@brief An inverted form of the std::optional<std::string> used to indicate if an operation was successful. 
		@details If Successful is true, Message is empty; else it contains an error message.
	*/
	struct report
	{
		//! @brief Indicates the operation worked
		bool Successful;
		//! @brief If Successful is false, this contains the error message
		std::string Message;
		//! An explicit boolean cast, allowing the report to be queried for success.
		explicit operator bool() const { return Successful; } 

		/// @brief An implicit cast for converting the object into an std::string, allowing the object to be treated as a string object wherever it can.
		/// @warning This is only meaningful if the user has first checked if Successful is false
		operator std::string() const { return Message; } 
	};

	/*!
	 * @brief The IO::Policy determines if a `harmless error' needs to be reported as a failure in a report (Strict), or treated as a success (Generous). 
	 	@details Examples of `harmless errors' include attempting to make a directory that already exists, or removing a file that does not exist
	 */
	enum Policy
	{
		Strict,
		Generous
	};

	//! A global policy used by functions when one is not manually supplied. Set to Strict by default. A user may override this by setting JSL::IO::DefaultPolicy = ?? , with the usual caveats of global variables.
	extern Policy DefaultPolicy;

	/*!
	 * @brief Attempts to create a directory at the path provided using ``std::filesystem::create_directories``.
	 	@details Will create any intermediary directories in the chain: ``mkdir a/b`` will create ``a`` if it does not exist, and then make ``b``.
	 * @param directory The target directory to create 
	 * @param policy The policy regarding extant directories: if the target already exists, this is treated as an error in Strict mode, but a success under a Generous policy. 
	 * @return An IO::report indicating success and any  error messages. 
	 */
	report mkdir(const std::filesystem::path directory, Policy policy = DefaultPolicy);

	/*!
	 * @brief Attempts to delete the file at the path provided using ``std::filesystem::remove_all``
	 	@details Always returns a failed state if the input is a directory; use removeDirectory instead.
	 * @param pathToFile The path to the file to be deleted
	 * @param policy The policy regarding missing files: if the target does not exist, a Strict policy returns a failure; a Generous policy treats this as a success.
	 * @return An IO::report indicating success and any error messages
	 */
	report remove(const std::filesystem::path pathToFile, Policy policy = DefaultPolicy);

	/*!
		@brief Attempts to remove the object at the path. If the target is a directory, it deletes all the contents.
		@details This is a wrapper around the remove function that forces a user to acknowledge they're deleting a whole directory and all its contents
		@param pathToDirectory The directory to delete
		@param policy The policy regarding non-directory targets and missing files. A strict policy returns a failure if the target is a file, or does not exist. A generous policy allows deletion of anything, and does not consider its absence to be an error.
	*/
	report removeDirectory(const std::filesystem::path pathToDirectory, Policy policy = DefaultPolicy);

}