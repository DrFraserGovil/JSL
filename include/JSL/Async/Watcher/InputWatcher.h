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
	// This has to be slightly more complicated (i.e. not just a trawl of cin) because we want to be able to interrupt mid-blocking call. That means going slightly deeper than just getline()
	class Input
	{
	  public:
		using callback = std::function<void(std::string)>;

		Input();
		explicit Input(callback fcn);

		void Initialise(callback fcn);
		~Input();

		void Start();
		void Stop();

		Input(const Input &) = delete;
		Input &operator=(const Input &) = delete;

		friend class Panopticon;

	  private:
		void GatherLines(const char *data, size_t len);

		void Run();
#ifdef WINMODE
		std::atomic<bool> ThreadExited = true;
#else
		int ShutdownPipe[2]{-1, -1};
#endif
		bool Initialised = false;
		callback Callback;
		std::atomic<bool> Running{false};
		std::thread WorkerThread;
		std::string LineBuffer;
	};
} // namespace JSL::Async::Watcher
