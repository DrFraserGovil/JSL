#pragma once
#if defined(_WIN32) || defined(_WIN64)
#define WINMODE
#endif
#include <atomic>
#include <functional>
#include <string>
#include <thread>
namespace JSL::Async::Watcher
{
	/*! @brief An Async::Watcher class that fires a callback function whenever a complete line is entered into the standard input
		@details It is slightly more complex than a blocking read() or std::getline() call because we want it to be interruptible
	 */
	class Input
	{
	  public:
		using callback = std::function<void(std::string)>;

		//! @brief Blank constructor: creates an object in an uninitialised state
		Input();

		/*! @brief Creates a Watcher which calls the provided function on every complete line passed to the standard input
			@param fcn A callback function which takes a line from the standard input, and acts on it. Additional arguments should be provided via a capturing lambda. The callback is executed on a parallel, non-blocking thread, so any data races must be manually handled by the user.
		  */
		explicit Input(callback fcn);

		/*! @brief A delayed initialisation (for blank-constructed watchers), or sets a new callback function
			@param fcn A callback function which takes a line from the standard input, and acts on it. Additional arguments should be provided via a capturing lambda. The callback is executed on a parallel, non-blocking thread, so any data races must be manually handled by the user.

		 */
		void Initialise(callback fcn);

		/*! @brief Destructor: if the watcher was not manually Stop()-ed, it is called here
		 */
		~Input();

		//! @brief A non-blocking function call which starts the process of watching cin. The callback is always executed on this parallel thread.
		//! @throws std::runtime_error If the object was not initialised with a callback function
		void Start();

		//! @brief Stops the parallel thread and joins it, halting any further callbacks.
		//! @details There may be a small gap in between the invocation of this function and the actual stoppage, where more events may occur.
		void Stop();

		//! Deleted for rule-of-3
		Input(const Input &) = delete;
		//! Deleted for rule-of-3
		Input &operator=(const Input &) = delete;

		//! Panopticon needs to access Initialised, and this was easier than having a Getter function
		friend class Panopticon;

	  private:
		//! @brief The asynchronous thread on which the Run() function is executed, after Start() is called
		std::thread WorkerThread;

		//! @brief The function loop used by the worker thread, where the actual watching happens.
		//! @details For POSIX systems, this amounts to a poll() call on STDIN, for Windows it is a blocking (but interruptible) call to ReadFile on the STD_INPUT_HANDLE
		void Run();

		//! @brief Collates the data into complete lines
		//! @details For posix systems (in non-piped-input mode), this is spurious as poll only fires when a complete line arrives, but for file redirection, or on Windows it's a bit more complex, so have to wrap it in this
		void GatherLines(const char *data, size_t len);

		//! @brief A buffer where incomplete lines are stored during GatherLines
		std::string LineBuffer;

#ifdef WINMODE
		//! @brief The signal used to indicate to windows that the worker thread has exited
		//! @details When Stop() is called, if this value is false, it repeatedly sends CancelSynchronousIo calls until the thread dies
		std::atomic<bool> ThreadExited = true;
#else
		//! @brief File Descriptors for the POSIX pipe used to signal to the worker thread that it is time to exit
		int ShutdownPipe[2]{-1, -1};
#endif
		//! @brief Set to True when Initialise() is called (or when the non-blank constructor runs).
		bool Initialised = false;

		//! @brief The storage location for the callback functions given during Initialisation; this is the function called on each line of the input
		callback Callback;

		//! @brief This is the value which is set to false to indicate to the workers to exit their loops
		std::atomic<bool> Running{false};
	};
} // namespace JSL::Async::Watcher
