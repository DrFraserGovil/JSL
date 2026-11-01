#include <JSL.h>
#include <JSL/Async/Socket.h>
#include <JSL/Async/Watcher.h>
#include <JSL/Log.h>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <condition_variable>
#include <vector>
using namespace JSL::Async;
namespace
{
	// Splices stdin to a private pipe for the duration of the test, so
	// InputWatcher's real (non-std::cin) read path can be driven
	// programmatically. This is a process-wide substitution -- one
	// splicer at a time, restored on destruction so later tests aren't
	// left reading from a dead pipe.
	class StdinSplicer
	{
	  public:
		StdinSplicer()
		{
#if defined(_WIN32) || defined(_WIN64)
			OriginalHandle = GetStdHandle(STD_INPUT_HANDLE);
			SECURITY_ATTRIBUTES sa{sizeof(SECURITY_ATTRIBUTES), nullptr, TRUE};
			CreatePipe(&ReadHandle, &WriteHandle, &sa, 0);
			SetStdHandle(STD_INPUT_HANDLE, ReadHandle);
#else
			OriginalFd = dup(STDIN_FILENO);
			int fds[2];
			pipe(fds);
			ReadFd = fds[0];
			WriteFd = fds[1];
			dup2(ReadFd, STDIN_FILENO);
#endif
		}
		~StdinSplicer()
		{
#if defined(_WIN32) || defined(_WIN64)
			SetStdHandle(STD_INPUT_HANDLE, OriginalHandle);
			CloseHandle(ReadHandle);
			CloseHandle(WriteHandle);
#else
			dup2(OriginalFd, STDIN_FILENO);
			close(OriginalFd);
			close(ReadFd);
			close(WriteFd);
#endif
		}
		StdinSplicer(const StdinSplicer &) = delete;
		StdinSplicer &operator=(const StdinSplicer &) = delete;

		void Write(const std::string &data)
		{
#if defined(_WIN32) || defined(_WIN64)
			DWORD written = 0;
			WriteFile(WriteHandle, data.data(), static_cast<DWORD>(data.size()), &written, nullptr);
#else
			write(WriteFd, data.data(), data.size());
#endif
		}

	  private:
#if defined(_WIN32) || defined(_WIN64)
		HANDLE OriginalHandle{}, ReadHandle{}, WriteHandle{};
#else
		int OriginalFd{-1}, ReadFd{-1}, WriteFd{-1};
#endif
	};

	// Reads one line from the process's standard input directly, as a program pausing the Input watcher would. On Windows the
	// StdinSplicer swaps only the Win32 stdin handle (SetStdHandle), which std::cin's C runtime never re-reads, so the line is read
	// from that handle, as the watcher itself does
	std::string ReadStdinLine()
	{
		std::string line;
#if defined(_WIN32) || defined(_WIN64)
		HANDLE input = GetStdHandle(STD_INPUT_HANDLE);
		char c = 0;
		DWORD got = 0;
		while (ReadFile(input, &c, 1, &got, nullptr) && got == 1 && c != '\n') line += c;
		if (!line.empty() && line.back() == '\r') line.pop_back();
#else
		std::getline(std::cin, line);
#endif
		return line;
	}

	// Accumulates every line delivered, so a test can wait for a specific
	// line/count without caring about exact delivery timing.
	struct LineCollector
	{
		std::mutex Mutex;
		std::condition_variable Cv;
		std::vector<std::string> Lines;

		auto Callback()
		{
			return [this](std::string line) {
				{
					std::lock_guard<std::mutex> lock(Mutex);
					Lines.push_back(std::move(line));
				}
				Cv.notify_one();
			};
		}

		bool WaitFor(const std::function<bool(const std::vector<std::string> &)> &pred, std::chrono::milliseconds timeout = std::chrono::seconds(2))
		{
			std::unique_lock<std::mutex> lock(Mutex);
			return Cv.wait_for(lock, timeout, [&] { return pred(Lines); });
		}
	};
} // namespace

TEST_CASE("Input Watcher", "[watcher][input]")
{
	SECTION("Basic Initialisation")
	{
		Watcher::Input Watcher([](auto) {});
		REQUIRE_NOTHROW(Watcher.Start());
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Receives a Single Line")
	{
		StdinSplicer splicer;
		LineCollector collector;
		Watcher::Input Watcher(collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());

		splicer.Write("hello world\n");

		bool found = collector.WaitFor([](auto &lines) { return !lines.empty(); });
		REQUIRE(found);
		REQUIRE(collector.Lines[0] == "hello world");
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Receives Multiple Lines in Order")
	{
		StdinSplicer splicer;
		LineCollector collector;
		Watcher::Input Watcher(collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());

		splicer.Write("first\nsecond\nthird\n");

		bool found = collector.WaitFor([](auto &lines) { return lines.size() >= 3; });
		REQUIRE(found);
		REQUIRE(collector.Lines == std::vector<std::string>{"first", "second", "third"});
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Handles a Line Split Across Multiple Writes")
	{
		// The specific case that would deadlock a getline()-based
		// implementation: the newline doesn't arrive in the same write as
		// the rest of the line.
		StdinSplicer splicer;
		LineCollector collector;
		Watcher::Input Watcher(collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());

		splicer.Write("partial-li");
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		splicer.Write("ne\n");

		bool found = collector.WaitFor([](auto &lines) { return !lines.empty(); });
		REQUIRE(found);
		REQUIRE(collector.Lines[0] == "partial-line");
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Can Restart After Stop")
	{
		StdinSplicer splicer;
		LineCollector collector;
		Watcher::Input Watcher(collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());
		REQUIRE_NOTHROW(Watcher.Stop());
		REQUIRE_NOTHROW(Watcher.Start());

		splicer.Write("after restart\n");

		bool found = collector.WaitFor([](auto &lines) { return !lines.empty(); });
		REQUIRE(found);
		REQUIRE(collector.Lines[0] == "after restart");
		REQUIRE_NOTHROW(Watcher.Stop());
	}

#if !defined(_WIN32) && !defined(_WIN64)
	SECTION("Restarting Does Not Leak File Descriptors")
	{
		// each Start() used to open a fresh shutdown pipe, closed only by the destructor
		auto openDescriptors = [] {
			std::filesystem::path folder = std::filesystem::exists("/proc/self/fd") ? "/proc/self/fd" : "/dev/fd";
			return std::distance(std::filesystem::directory_iterator(folder), std::filesystem::directory_iterator{});
		};
		StdinSplicer splicer;
		Watcher::Input Watcher([](auto) {});
		REQUIRE_NOTHROW(Watcher.Start());
		REQUIRE_NOTHROW(Watcher.Stop());
		auto before = openDescriptors();
		for (int i = 0; i < 50; ++i)
		{
			REQUIRE_NOTHROW(Watcher.Start());
			REQUIRE_NOTHROW(Watcher.Stop());
		}
		REQUIRE(openDescriptors() == before);
	}
#endif

	SECTION("Stop Does Not Hang With No Input Pending")
	{
		// The case that would hang forever if Stop() couldn't interrupt a
		// blocked wait: the watcher is started but nothing is ever written.
		StdinSplicer splicer;
		Watcher::Input Watcher([](auto) {});
		REQUIRE_NOTHROW(Watcher.Start());
		REQUIRE_NOTHROW(Watcher.Stop());
	}
}
