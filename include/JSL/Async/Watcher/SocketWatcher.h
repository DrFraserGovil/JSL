#pragma once
#include <JSL/Async/Socket/Listener.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace JSL::Async::Watcher
{
	//! @brief An Async::Watcher class that fires a callback function whenever a message is recieved to the watched Socket
	class Socket
	{
	  public:
		using callback = std::function<void(std::string)>;

		//! @brief Blank constructor: creates an object in an uninitialised state
		Socket();

		/*! @brief Creates a Watcher which calls the provided function on data recieved by the named socket
			@param socketName The Identifier of the socket (not the path to the socketfile) which will be monitored
			@param fcn A callback function which processes each socket message. Additional arguments should be provided via a capturing lambda. The callback is executed on a parallel, non-blocking thread, so any data races must be manually handled by the user.
			@param forceAcquire If true, will send an exit command to any other process currently monitoring the named socket
			@throws std::runtime_error if the associated Async::Socket constructor throws an error
		  */
		explicit Socket(std::string_view socketName, callback fcn, bool forceAcquire = false);

		/*! @brief A delayed initialisation (for blank-constructed watchers), or sets a new callback function
			@param socketName The Identifier of the socket (not the path to the socketfile) which will be monitored
			@param fcn A callback function which processes each socket message. Additional arguments should be provided via a capturing lambda. The callback is executed on a parallel, non-blocking thread, so any data races must be manually handled by the user.
			@param forceAcquire If true, will send an exit command to any other process currently monitoring the named socket
			@throws std::runtime_error if the associated Async::Socket constructor throws an error
		  */
		void Initialise(std::string_view socketName, callback fcn, bool forceAcquire = false);

		//! @brief Calls Stop() and closes the Socket::Listener if they remain open
		~Socket();

		//! @brief A non-blocking function call which starts the process of watching cin. The callback is always executed on this parallel thread.
		//! @throws std::runtime_error If the object was not initialised with a callback function or target socket
		void Start();

		//! @brief Stops the parallel thread and joins it, halting any further callbacks.
		//! @details There may be a small gap in between the invocation of this function and the actual stoppage, where more events may occur.
		void Stop();

		//! Panopticon needs to access Initialised, and this was easier than having a Getter function
		friend class Panopticon;

		/*! Sets the value of the Listener timeout duration
			@param time The length of time a socket has to provide the data, before a Timeout is returned
		*/
		void SetTimeout(std::chrono::milliseconds time);

	  private:
		//! @brief The asynchronous thread on which the Run() function is executed, after Start() is called
		std::thread WorkerThread;

		//! @brief The function loop used by the worker thread, where the actual watching happens.
		//! @details For POSIX systems, this amounts to a poll() call on STDIN, for Windows it is a blocking (but interruptible) call to ReadFile on the STD_INPUT_HANDLE
		void Run();

		//! @brief Set to True when Initialise() is called (or when the non-blank constructor runs).
		bool Initialised = false;

		//! @brief The storage location for the callback functions given during Initialisation; this is the function called on each line of the input
		callback Callback;

		//! @brief This is the value which is set to false to indicate to the workers to exit their loops
		std::atomic<bool> Running{false};

		//! The object which performs the reads of the Socket
		JSL::Async::Socket::Listener ListenerInstance;

		//! The length of time a socket has to provide the data, before a Timeout is returned
		std::chrono::milliseconds PollTimeout{50};

		//! Stores exceptions safely so we don't get an std::terminate() if an error is thrown in a parallel thread
		std::exception_ptr StoredException = nullptr;

		//! Rule-of-3 deletion
		Socket(const Socket &) = delete;
		//! Rule-of-3 deletion
		Socket &operator=(const Socket &) = delete;
	};
} // namespace JSL::Async::Watcher
