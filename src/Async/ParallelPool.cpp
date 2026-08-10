#include <JSL/Async/ParallelPool.h>
#include <JSL/Log.h>
#include <JSL/internal/error.h>
namespace JSL::Async
{
	Pool::Pool(size_t ncores)
	{
		if (ncores == 0)
		{
			LOG(WARN) << "Cannot create a pool with 0 workers. 1 worker established";
			ncores = 1;
		}

		Workers.reserve(ncores);
		WorkersRunning = true;
		for (size_t w = 0; w < ncores; ++w)
		{
			Workers.emplace_back(&Pool::WorkerFunction, this);
		}
		PendingTasks = 0;
	}

	Pool::~Pool()
	{
		if (PendingTasks > 0)
		{
			LOG(WARN) << "Pool destroyed with " << PendingTasks << " tasks still pending.\n Call Synchronise before pool goes out of scope to prevent this issue";
		}
		WorkersRunning = false; // atomic so no lock needed

		TaskReady.notify_all();

		for (auto &w : Workers)
		{
			if (w.joinable())
			{
				w.join();
			}
		}
	}

	void Pool::Synchronise()
	{
		// This creates a lock but does not leave it locked (so no deadlocks)

		std::unique_lock lock(Sync);
		TaskComplete.wait(lock, [&] {
			return (PendingTasks == 0);
		});

		if (ErrorMessage)
		{
			std::exception_ptr ex = nullptr;
			std::swap(ex, ErrorMessage);
			std::rethrow_exception(ex);
		}
	}

	void Pool::AsyncTask(std::function<void()> func)
	{
		++PendingTasks;
		{
			std::unique_lock lock(Queue);
			TaskQueue.push(func);
		}

		// if nobody is waiting to read this, it doesn't matter as the sleep predicate means it is captured implicitly
		TaskReady.notify_one();
	}

	void Pool::WorkerFunction()
	{
		std::function<void()> currentTask; // establish outside of scope

		while (true)
		{
			{
				std::unique_lock lock(Queue);

				// this predicate makes sure the worker wakes up *even if it missed a cv-wakeup call*
				TaskReady.wait(lock, [this] { return !WorkersRunning || !TaskQueue.empty(); });

				// exit point from this function
				if (!WorkersRunning)
				{
					return;
				}

				currentTask = std::move(TaskQueue.front());
				TaskQueue.pop();
			}

			// now execute the task we just saved
			// we do it this way so that the task executres *after* the lock scope has closed
			try
			{
				currentTask();
				// inform the scheduler that we've done it
				{
					// we mutex lock here even though PendingTasks is atomic to prevent a race condition in the dispatcher acquiring this lock
					std::lock_guard<std::mutex> lock(Sync);
					--PendingTasks;
				}
			}
			catch (...)
			{
				LOG(ERROR) << "Exception thrown during parallel execution";
				std::unique_lock lock(Sync);
				if (!ErrorMessage) { ErrorMessage = std::current_exception(); } // capture the first msg, not the last
				--PendingTasks;
			}

			TaskComplete.notify_one();
		}
	}

	void Pool::LoopTask(size_t nLoop, std::function<void(size_t)> func, Pool::DistributionPolicy policy)
	{
		size_t N = Workers.size();

		switch (policy)
		{
			case Pool::DistributionPolicy::Sequential: {
				for (size_t w = 0; w < N; ++w)
				{
					AsyncTask([nLoop, func, N, w]() {
						for (size_t i = w; i < nLoop; i += N)
						{
							func(i);
						}
					});
				}
				break;
			}
			case Pool::DistributionPolicy::Balanced: {
				auto next = std::make_shared<std::atomic<size_t>>(0);

				for (size_t w = 0; w < N; ++w)
				{
					AsyncTask([next, nLoop, func]() {
						size_t i;
						while ((i = (*next)++) < nLoop)
						{
							func(i);
						}
					});
				}
				break;
			}

			default: {
				JSL::internal::FatalError("Unknown policy", JSL_LOCATION) << "Unknown loop distribution policy";
			}
		}
	}

} // namespace JSL::Async
