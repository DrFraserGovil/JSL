#include <JSL/Async/Watcher/InputWatcher.h>
#include <JSL/internal/error.h>

#include <atomic>
#include <string>
#include <thread>

#ifdef WINMODE
#include <chrono>
#include <iostream>
#include <windows.h>
#else
#include <cerrno>
#include <fcntl.h>
#include <poll.h>
#include <unistd.h>
#endif
namespace JSL::Async::Watcher
{
	Input::Input()
	{
		Initialised = false;
	}
	Input::Input(callback fcn)
	{
		Initialise(fcn);
	}

	void Input::Initialise(callback fcn)
	{
		Callback = fcn;
#ifndef WINMODE
		if (::pipe(ShutdownPipe) == 0) // opens a new pipe, and saves the values into ShutdownPipe
		{
			::fcntl(ShutdownPipe[0], F_SETFL, O_NONBLOCK);
			::fcntl(ShutdownPipe[1], F_SETFL, O_NONBLOCK);
		}
		else
		{
			JSL::internal::LibraryError("Invalid pipe", JSL_LOCATION) << "Could not create a pipe for the InputWatcher";
		}
#endif
		Initialised = true;
	}

	Input::~Input()
	{
		if (!Initialised) return;
		Stop();
#ifndef WINMODE
		::close(ShutdownPipe[0]);
		::close(ShutdownPipe[1]);
#endif
	}
	void Input::Start()
	{
		if (!Initialised)
		{
			JSL::internal::LibraryError("Uninitialised watcher", JSL_LOCATION) << "Cannot Start() an uninitialised Watcher::Input";
		}
		if (Running.exchange(true)) return; // if already running, return, otherwise continue

		LineBuffer = ""; // clear it from previous iterations
						 // on windows, have to store the 'running' status as an atomic (on posix, we can implicitly shut it down)
#ifdef WINMODE
		ThreadExited.store(false, std::memory_order_release);
#endif

		// spawn the actual watcher process on a new thread
		WorkerThread = std::thread(&Input::Run, this);
	}
	void Input::Stop()
	{
		if (!Running.exchange(false)) return; // if not running, do nothing

#ifdef WINMODE
		HANDLE nativeHandle = static_cast<HANDLE>(WorkerThread.native_handle());

		// we ping the worker thread with "cancel" commands
		// might have to ping multiple times if the worker is not dormant
		while (!ThreadExited.load(std::memory_order_acquire))
		{
			// cancels any currently-waiting ReadFile calls
			CancelSynchronousIo(nativeHandle);
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
		if (WorkerThread.joinable()) WorkerThread.join();

		// A cancelled/failed read can leave std::cin (if ever mixed in elsewhere)
		std::cin.clear();
#else
		// write some data to the ShutdownPipe -- the worker in Run() will see this and exit
		uint8_t signal = 1;
		(void)::write(ShutdownPipe[1], &signal, sizeof(signal));

		// wait for the joining
		if (WorkerThread.joinable()) WorkerThread.join();

		// drain the ShutdownPipe so it is empty for future calls
		char dummy[128];
		while (::read(ShutdownPipe[0], dummy, sizeof(dummy)) > 0) {}
#endif
	}
	void Input::GatherLines(const char *data, size_t len)
	{
		LineBuffer.append(data, len);
		size_t pos;
		while ((pos = LineBuffer.find('\n')) != std::string::npos)
		{
			std::string line = LineBuffer.substr(0, pos);
			if (!line.empty() && line.back() == '\r') line.pop_back();
			LineBuffer.erase(0, pos + 1);

			try
			{
				Callback(std::move(line));
			}
			catch (...)
			{
				JSL::internal::LibraryError("Callback failure", JSL_LOCATION) << "The cin-callback threw an exception while processing ' " << line << "'";
			}
		}
	}

#ifdef WINMODE
	// WINDOWS IMPLEMENTATION OF THE WORKER LOOP
	void Input::Run()
	{
		HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE);
		char buf[4096];

		// loop while Running remains true. Running is the global command, indicating what should be happening
		while (Running.load(std::memory_order_acquire))
		{
			// Reset the ThreadExited (the local tracker, indicating what *is* happening), so that Stop() knows we're still running
			ThreadExited.store(false, std::memory_order_release);

			// quick escape hatch in case we were ordered to stop between the while check and here
			if (!Running.load(std::memory_order_acquire)) break;

			// activate the blocking ReadFile on the CIN
			DWORD bytesRead = 0;
			BOOL ok = ReadFile(hStdin, buf, sizeof(buf), &bytesRead, nullptr);

			if (!ok || bytesRead == 0)
			{
				// Interpret any faulty read (which might be due to external errors, or from CancelSynchronousIo) as a "stop" command
				break;
			}
			GatherLines(buf, static_cast<size_t>(bytesRead));
		}
		ThreadExited.store(true, std::memory_order_release);
	}

#else
	// POSIX IMPLEMENTATION OF THE WORKER LOOP
	void Input::Run()
	{
		// our polling is over two streams: the CIN (0) and the ShutdownPipe (1)
		//
		struct pollfd fds[2] = {};
		fds[0].fd = STDIN_FILENO;
		fds[0].events = POLLIN;
		fds[1].fd = ShutdownPipe[0];
		fds[1].events = POLLIN;

		char buf[4096];

		// loop while Running remains true. Running is the global command, indicating what should be happening
		while (Running.load(std::memory_order_acquire))
		{
			int ret = ::poll(fds, 2, -1);
			if (ret < 0)
			{
				// poll might have spurious wakeup(EINTR), if so, just reloop. Otherwise, exit.
				if (errno == EINTR) continue;
				break;
			}
			if (fds[1].revents & POLLIN) break; // shutdown requested -- any signal sent to fds[1] is a shutdown

			// this is a standard cin in query
			if (fds[0].revents & (POLLIN | POLLHUP))
			{
				ssize_t n = ::read(STDIN_FILENO, buf, sizeof(buf));
				if (n <= 0)
				{
					// 0 = EOF, <0 = real error (EAGAIN shouldn't happen
					// right after POLLIN, but treat conservatively).
					break;
				}
				GatherLines(buf, static_cast<size_t>(n));
			}
		}
	}

#endif
}; // namespace JSL::Async::Watcher
