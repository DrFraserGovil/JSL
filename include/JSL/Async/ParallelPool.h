#pragma once

#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>

namespace JSL::Async
{
	//! @brief Determine how tasks are assigned to workers during a tight Pool::LoopTask
	enum class DistributionPolicy
	{
		Sequential, //!< Interleaved assignment; best for tight loops where execution time is independent of index
		Balanced	//!< Atomic sequential assignment; best for slower loops, where execution time can vary
	};

	template <typename T>
	concept VectorStorable = std::default_initializable<T> && std::movable<T>;

	/*!
		Spins up a bunch of workers which wait for new asynchronous tasks to be given to them.
	*/
	class Pool
	{
	  public:
		/*! @brief Construct a parallel pool with a number of worker threads spun up, awaiting tasks
			@param ncores The number of worker threads to be spun up (does not include the 'main' thread)
		 */
		Pool(size_t ncores);

		/*! @brief Destroys the pool. In-progress tasks are completed, but tasks in the queue are abandoned (with a warning message)
			@warning It is almost always better to call Synchronise() before this destructor
			@details May take a small delay to complete, as the worker threads must complete their in-flight tasks before they get the signal to shut down. Any queued tasks are dropped.
		 */
		~Pool();

		/*! @brief Adds the task into a queue to be executed by asynchronous threads
			@details Manages their own internal memory safety, but any mutable variables passed by the user should be considered unsafe unless manually mutex-guarded
			@param func The function to execute. Must be a function with void() signature -- if arguments are required, wrap the function in a lambda: func = [=](){original(args)}; if the arguments are large/non-copyable objects which are guaranteed to have lifetimes which persist until Synchronise is called, you may capture by reference ([&]), but this is dangerous.
		*/
		void AsyncTask(std::function<void()> func);

		/*! @brief Adds the task into a queue to be executed by asynchronous threads, and returns a promise of the return value
			@details Manages their own internal memory safety, but any mutable variables passed by the user should be considered unsafe unless manually mutex-guarded
			@param func The function to execute. As with AsyncTask, arguments must be wrapped in a capturing lambda -- unlike AsyncTask, return values are permitted.
			@return An std::future to the return value of the task. When the task completes, this value can be retrieved by .get(). The user may either manually wait per-tasks for the value to be filled (using .wait() on the future), or all values are guaranteed to be valid after calling Synchronise()
			@warning If the function throws an exception, the std::future will have set_exception called on it, which will throw an exception when .get() is called
		*/
		template <class T>
		std::future<T> AsyncReturn(std::function<T()> func)
		{
			auto promise = std::make_shared<std::promise<T>>();
			std::future<T> future = promise->get_future();

			AsyncTask([promise, func]() {
				try
				{
					promise->set_value(func());
				}
				catch (...)
				{
					promise->set_exception(std::current_exception());
				}
			});
			return future;
		}

		/*! @brief Executes a set number of copies of the task. A more efficient alternative to for(int i =0; i < nLoop; ++i){func(i);}; Synchronise();.
			@description The efficiency gains occur due to the 'chunk assignment' of tasks under the Sequential policy, which removes task-acquisition overhead. Blocks until the loop is completed.
			@param nLoop the number of iterations to add into the queue
			@param func The function to be computed. As with AsyncTask, arguments that are not the loop index must be wrapped in a capturing lambda.
			@param policy The DistributionPolicy assigned to the dispatcher
		 */
		void LoopTask(size_t nLoop, std::function<void(size_t)> func, DistributionPolicy policy = DistributionPolicy::Sequential);

		/*! @brief Executes a set number of copies of the task, and stores the return values in an output vector. Blocks until the loop is completed as if Synchronise() had been called
			@param nLoop the number of iterations to add into the queue
			@param func The function to be computed. As with AsyncTask, arguments that are not the loop index must be wrapped in a capturing lambda.
			@param policy The DistributionPolicy assigned to the dispatcher
			@returns A vector where out[i] = func(i)
		*/
		template <VectorStorable T>
		std::vector<T> LoopReturn(size_t nLoop, std::function<T(size_t)> func, DistributionPolicy policy = DistributionPolicy::Sequential)
		{
			std::vector<T> out(nLoop);

			LoopReturn(out, func, policy);
			return out;
		}

		/*! @brief Executes a set number of copies of the task, and stores the return values in a vector provided by the user
			@param holder A vector which will store the output results according to out[i] = func(i)
			@param nLoop the number of iterations to add into the queue
			@param func The function to be computed. As with AsyncTask, arguments that are not the loop index must be wrapped in a capturing lambda.
			@param policy The DistributionPolicy assigned to the dispatcher
		*/
		template <VectorStorable T>
		void LoopReturn(std::vector<T> &holder, std::function<T(size_t)> func, DistributionPolicy policy = DistributionPolicy::Sequential)
		{
			size_t nLoop = holder.size();

			LoopTask(nLoop, [&holder, func](auto idx) { holder[idx] = std::move(func(idx)); }, policy);

			Synchronise();
		}

		/*! @brief Blocks until all tasks in the queue have been completed
			@throws Exception: The first exception thrown by any task since the last Syncrhonisation effort, unless the task was an AsyncReturn, in which case the std::future wraps the error message (Which will be thrown when .get() is called)
		 */
		void Synchronise();

		//! Deleted copy constructor (threads are non copyable)
		Pool(const Pool &) = delete;
		//! Deleted copy assignment (threads are non moveable)
		Pool &operator=(const Pool &) = delete;
		//! Deleted move constructor (threads are non moveable)
		Pool(Pool &&) = delete;
		//! Deleted move assignment (threads are non moveable)
		Pool &operator=(Pool &&) = delete;

	  private:
		//! This is the vector where the workers live.
		std::vector<std::thread> Workers;

		//! The set of tasks which have been assigned, but not yet taken up by a worker
		std::queue<std::function<void()>> TaskQueue;

		//! Used to signal worker -> scheduler that a task has been completed
		std::condition_variable TaskComplete;

		//! Used to signal scheduler -> worker that a new task is ready
		std::condition_variable TaskReady;

		//! Used to store the results of try-catch exceptions until the next Synchronise call
		std::exception_ptr ErrorMessage;

		//! mutex for locking things pulling from /adding to the queue
		std::mutex Queue;

		//! mutex for syncing up the threads
		std::mutex Sync;

		//! An atomic count equal to TaskQueue.size()
		std::atomic<size_t> PendingTasks = 0;

		//! A flag used to indicate to all workers that the pool is being destroyed, and that they should exit the WorkerFunction
		std::atomic<bool> WorkersRunning = false;

		//! The main execution block for the workers: the Workers object is initialised with threads running this function.
		//! @details If no tasks in the queue, then this function uses condition_variables to sleep (i.e. taking up next to no CPU)
		void WorkerFunction();
	};

} // namespace JSL::Async
