#pragma once

#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <memory>
#include <future>

namespace JSL::Parallel
{
	
	template<typename T>
	concept VectorStorable = std::default_initializable<T> && std::movable<T>;

	/*!
		Spins up a bunch of workers which wait for new asynchronous tasks to be given to them.
	*/
	class Pool
	{
		public:
			Pool(size_t ncores);
			~Pool();

			//! @brief Adds the task into a queue to be executed by asynchronous threads
			//! @details Manages their own internal memory safety, but any mutable variables passed by the user should be considered unsafe unless manually mutex-guarded
			void AsyncTask(std::function<void()> func);

			//! @brief Adds a task into the queue and establishes an std::future pointing towards the returned value
			template<class T>
			std::future<T> AsyncReturn(std::function<T()> func)
			{
				auto promise = std::make_shared<std::promise<T>>();
				std::future<T> future = promise->get_future(); 

				AsyncTask([promise,func](){
					promise->set_value(func());
				});
				return future;
			}

			enum class DistributionPolicy
			{
				Sequential, //!< Interleaved assignment; best for tight loops where execution time is independent of index
				Balanced //!< Atomic sequential assignment; best for slower loops, where execution time can vary 
			};
			void LoopTask(size_t nLoop, std::function<void(size_t)> func, DistributionPolicy policy = DistributionPolicy::Sequential);
			
			
			//! A blocking function that asychronously fills the elements of a vector with the output types
			template<VectorStorable T>
			std::vector<T> LoopReturn(size_t nLoop, std::function<T(size_t)> func, DistributionPolicy policy = DistributionPolicy::Sequential)
			{
				std::vector<T> out(nLoop);

				LoopReturn(out,func,policy);
				return out;
			}

			//! A blocking function that asychronously fills the elements of a vector with the output types
			template<VectorStorable T>
			void LoopReturn(std::vector<T> & holder, std::function<T(size_t)> func, DistributionPolicy policy = DistributionPolicy::Sequential)
			{
				size_t nLoop = holder.size();

				LoopTask(nLoop,[&holder, func](auto idx){
					holder[idx] = std::move(func(idx));
				},policy);

				Synchronise();

			}

			//blocks until all tasks have been completed
			void Synchronise();

			// --- Delete copy/move constructors and assignment operators ---
			Pool(const Pool&) = delete;
			Pool& operator=(const Pool&) = delete;
			Pool(Pool&&) = delete;
			Pool& operator=(Pool&&) = delete;


		private:

			std::vector<std::thread> Workers;
			std::queue<std::function<void()>> TaskQueue;
			std::condition_variable TaskComplete;// worker -> scheduler 
			std::condition_variable TaskReady; // scheduler -> worker 

			std::mutex Queue;
			std::mutex Sync;
			std::atomic<size_t> PendingTasks = 0;
			std::atomic<bool> WorkersRunning = false;
			void WorkerFunction(int id);
	};



}