#pragma once
#include <JSL/Async/Socket/Listener.h>
#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <thread>

namespace JSL::Async::Watcher
{
	class Socket
	{
	  public:
		using callback = std::function<void(std::string)>;

		Socket();
		explicit Socket(std::string_view socketName, callback fcn, bool forceAcquire = false);

		void Initialise(std::string_view socketName, callback fcn, bool forceAcquire = false);
		~Socket();

		void Start();
		void Stop();

		Socket(const Socket &) = delete;
		Socket &operator=(const Socket &) = delete;

		friend class Panopticon;

	  private:
		void Run();

		bool Initialised = false;
		callback Callback;
		std::atomic<bool> Running{false};
		std::thread WorkerThread;
		JSL::Async::Socket::Listener ListenerInstance;
		std::chrono::milliseconds PollTimeout{50};
		std::exception_ptr StoredException = nullptr;
	};
} // namespace JSL::Async::Watcher
