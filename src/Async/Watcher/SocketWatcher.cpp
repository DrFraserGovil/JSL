#include <JSL/Async/Watcher/SocketWatcher.h>
#include <JSL/internal/error.h>

namespace JSL::Async::Watcher
{
	Socket::Socket()
	{
		Initialised = false;
	}

	Socket::Socket(std::string_view socketName, callback fcn, bool forceAcquire)
	{
		Initialise(socketName, fcn, forceAcquire);
	}

	void Socket::Initialise(std::string_view socketName, callback fcn, bool forceAcquire)
	{
		Callback = fcn;
		ListenerInstance.Initialise(socketName, forceAcquire);
		Initialised = true;
	}

	Socket::~Socket()
	{
		if (!Initialised) return;
		Stop();
		ListenerInstance.Close();
	}

	void Socket::Start()
	{
		if (!Initialised)
		{
			JSL::internal::LibraryError("Uninitialised watcher", JSL_LOCATION) << "Cannot Start() an uninitialised Watcher::Socket";
		}
		if (Running.exchange(true)) return;

		WorkerThread = std::thread(&Socket::Run, this);
	}

	void Socket::Stop()
	{

		if (!Running.exchange(false)) return;

		if (WorkerThread.joinable())
		{
			WorkerThread.join();
		}

		if (StoredException)
		{
			std::exception_ptr ex = std::move(StoredException);
			StoredException = nullptr;
			std::rethrow_exception(ex);
		}
	}
	void Socket::SetTimeout(std::chrono::milliseconds time)
	{

		PollTimeout = time;
	}

	void Socket::Run()
	{
		size_t consecutiveErrors = 0;
		constexpr size_t maxConsecutiveErrors = 5;
		try
		{
			while (Running.load(std::memory_order_acquire))
			{
				auto result = ListenerInstance.Read(PollTimeout);

				switch (result.Status)
				{
					case JSL::Async::Socket::ReadStatus::Success:
						consecutiveErrors = 0;
						Callback(std::move(result.Message));
						break;
					case JSL::Async::Socket::ReadStatus::TimedOut:
						consecutiveErrors = 0;
						break;
					default:
						consecutiveErrors += 1;
						// Throttle briefly on genuine errors to prevent tight spinning
						std::this_thread::sleep_for(std::chrono::milliseconds(10));
						break;
				}

				if (consecutiveErrors >= maxConsecutiveErrors)
				{
					throw std::runtime_error("Too many errors in socket read");
				}
			}
		}
		catch (...)
		{
			StoredException = std::current_exception();
		}
	}

} // namespace JSL::Async::Watcher
