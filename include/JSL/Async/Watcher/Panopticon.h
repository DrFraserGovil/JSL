#pragma once
#include "FileWatcher.h"
#include "InputWatcher.h"
#include "SocketWatcher.h"
#include <atomic>
#include <condition_variable>
#include <deque>
#include <map>
#include <optional>
#include <variant>
namespace JSL::Async::Watcher
{
	// The All-Seer

	namespace internal
	{
		//! @brief The data type used in the Panopticon to store events which have been processed by a Watcher.
		//! @details Each time a watcher records an event, they create an Instruction, and insert it into the queue for the Panopticon to process
		struct Instruction
		{
			enum class Type
			{
				//! Instruction from a Watcher::Input
				CIN,
				//! Instruction from a Watcher::File
				FILE,
				//! Instruction from a Watcher::Socket
				SOCKET,
				//! Instruction from a Panopticon::Stop() command
				SHUTDOWN
			};
			//! Indicates which type of watcher generated the result
			Type Origin;

			//! The identifier of the watcher which generated the result (the socketname for SOCKET, directory for FILE)
			std::string ID;

			//! The data associated with the instruction (string for CIN + SOCKET, FileChange for FILE)
			std::variant<std::string, std::set<FileChange>> Data;
		};
	} // namespace internal

	//! @brief The All-Seer; a manager class for multiple Async::Watcher objects, acting as a central orchestrator for a multi-input system
	class Panopticon
	{
	  public:
		//! callbacks which take a single string argument (for CIN or SOCKET instructions)
		using strCallBack = std::function<void(std::string)>;

		//! callbacks which take a whole batch of files (for FILE instructions)
		using batchCallBack = std::function<void(std::set<FileChange>)>;

		//! callbacks which take a single file change (for FILE instructions)
		using fileCallBack = std::function<void(FileChange)>;

		/*! @brief Activates the Watcher::Input module, and assigns the callback function to process any data passed to cin
			@param fcn The callback function to be assigned. This function will act on the main Panopticon thread (not on the Watcher::Input thread)
			@param exitString If not nullopt, the Watcher::Input will automatically call Stop() if this string is provided by the user.
			@param overwriteExisting Determines if an overwrite call generates an exception
			@throws std::runtime_error If this function is called after a callback has already been assigned; unless overwriteExisting is set to true
			@throws std::runtime_error If this function is called after Start() has been called, but before Stop()
		 */
		void SetInputCallback(strCallBack fcn, std::optional<std::string> exitString = std::nullopt, bool overwriteExisting = false);

		/*! @brief Activates the Watcher::Socket module, and assigns the callback function to process any data passed to the named socket
			@details Multiple sockets may be watched at once by calling this function multiple times with different socketIDs. The callbacks are stored per-ID.
			@param socketID The name of the socket (not the path to the socketfile) which is to be monitored
			@param fcn The callback function to be assigned to output from the named socket. This function will act on the main Panopticon thread (not on the Watcher::Socket thread)
			@param forceAcquire If true, runs the Socket forceAcquire module in the event that a socket with this ID already exists
			@param overwriteExisting Determines if an overwrite call generates an exception
			@throws std::runtime_error If this function is called after a callback has already been assigned to this socket; unless overwriteExisting is set to true
			@throws std::runtime_error If this function is called after Start() has been called, but before Stop()
		  */
		void SetSocketCallback(std::string socketID, strCallBack fcn, bool forceAcquire = false, bool overwriteExisting = false);

		/*! @brief Activates the Watcher::File module, and assigns the callback function to process the full batch of files passed to the named directory
			@details Multiple directories may be watched at once by calling this function multiple times with different names. The callbacks are stored per-directory.
			@param watchedDirectory The name of the directory which is to be monitored
			@param recursive If true, all child directories will also be monitored for changes
			@param fcn The callback function to be assigned to output from the directory. This function will act on the main Panopticon thread (not on the Watcher::File thread)
			@param overwriteExisting Determines if an overwrite call generates an exception
			@throws std::runtime_error If this function is called after a callback has already been assigned to this directory; unless overwriteExisting is set to true
			@throws std::runtime_error If this function is called after Start() has been called, but before Stop()
			@warning There is no check if a directory is watched as a result of being a child of another directory which is watched. This may result in events being 'double counted'. The user is responsible for ensuring that if they watch multiple directories, that there is no overlap
		  */
		void SetFileBatchCallback(std::string watchedDirectory, bool recursive, batchCallBack fcn, bool overwriteExisting = false);

		/*! @brief An alternative to the SetFileBatchCallback, where the callback function is per-file, rather than for the whole batch. It is otherwise identical in function: Activates the Watcher::File module, and assigns the callback function on each file passed to the named directory
			@details Multiple directories may be watched at once by calling this function multiple times with different names. The callbacks are stored per-directory.
			@param watchedDirectory The name of the directory which is to be monitored
			@param recursive If true, all child directories will also be monitored for changes
			@param fcn The callback function to be assigned to output from the directory. This function will act on the main Panopticon thread (not on the Watcher::File thread)
			@param overwriteExisting Determines if an overwrite call generates an exception
			@throws std::runtime_error If this function is called after a callback has already been assigned to this directory; unless overwriteExisting is set to true
			@throws std::runtime_error If this function is called after Start() has been called, but before Stop()
		  */
		void SetSingleFileCallback(std::string watchedDirectory, bool recursive, fileCallBack fcn, bool overwriteExisting = false);

		/*! A blocking function which activates all initialised Watchers, and executes any tasks placed into the queue. Runs until a SHUTDOWN instruction is provided by one of the threads.
			@details The SetInputCallback provides a default way to terminate this loop, via the exitString. Otherwise the user must provide some means of calling Stop() within one of the callback functions, or from another asynchronous function.
			@throws std::runtime_error If Start() has already been called, but has not yet been Stop()ed
		 */
		void Start();

		//! @brief Sends a SHUTDOWN instruction to the queue, before causing the Start() loop to exit
		//! @details Any instructions which were in the queue beforehand will be completed, but any which arrive after are discared.
		//! @details This does not wait for the worker threads of the Watchers to sync up; the Synchronisation occurs during Shutdown(), which is a side effect of this function, but not explicitly called.
		void Stop();

	  private:
		//! Resource cleanup which is triggered when the main loop recieves a SHUTDOWN instruction.
		void Shutdown();

		//! The mutex used to prevent race conditions when placing objects into or reading from the instruction queue
		std::mutex Queue;

		//! The condition variable used to send the main thread to sleep until there is data in the queue
		std::condition_variable AwaitingInstruction;

		//! @brief The online-queue into which the Watchers insert instructions.
		std::deque<internal::Instruction> Instructions = {};

		//! A flag used to indicate that Start() has been called (but not Stop()), and therefore disables assigning new callbacks
		std::atomic<bool> IsRunning{false};

		//! The Watcher::Input module -- initialised into a blank state (and therefore unused) until a callback is provided
		Watcher::Input InputTracker{};

		//! The callback applied to instructions recieved from the InputTracker module
		strCallBack cinCallback;

		//! A set of Watcher::Socket modules, each associated with a unique socketID
		std::map<std::string, std::unique_ptr<Watcher::Socket>> SocketTracker{};

		//! A map of callbacks, each such that socketCallback[id] corresponds to the callback applied to SocketTracker[id] when it pushes an instruction to the queue
		std::map<std::string, strCallBack> socketCallback{};

		//! A set of Watcher::File modules, each associated with a unique directory RootPath
		std::map<std::string, std::unique_ptr<Watcher::File>> FileTracker{};

		//! A map of callbacks, each such that fileCallback[id] corresponds to the callback applied to FileTracker[id] when it pushes an instruction to the queue
		std::map<std::string, batchCallBack> fileCallback{};
	};
}; // namespace JSL::Async::Watcher
