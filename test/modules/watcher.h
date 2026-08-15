

#include "../test_utils/catch_extended.h"
#include "catch2/matchers/catch_matchers_string.hpp"
#include <JSL.h>
#include <JSL/Async/Socket.h>
#include <JSL/Async/Watcher.h>
#include <JSL/Log.h>
#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <numeric>
#include <vector>
using namespace JSL::Async;

TEST_CASE("Socket Watcher", "[watcher][socket]")
{
	std::string socketName = "test.sock";

	SECTION("Basic Initiliasation")
	{
		JSL::Async::Watcher::Socket Watcher;
		REQUIRE_NOTHROW(Watcher.Initialise(socketName, [&](auto msg) {}));
		REQUIRE_NOTHROW(Watcher.Start());
		REQUIRE_NOTHROW(Watcher.Stop());
	}
	SECTION("Watcher Throws on Duplicate Names")
	{
		JSL::Async::Watcher::Socket Watcher(socketName, [&](auto msg) {});
		JSL::Async::Watcher::Socket clash;
		REQUIRE_THROWS(clash.Initialise(socketName, [&](auto msg) {}));
	}
	SECTION("Watcher Releases resources when out of Scope")
	{
		{
			JSL::Async::Watcher::Socket Watcher;
			REQUIRE_NOTHROW(Watcher.Initialise(socketName, [&](auto msg) {}));
		}
		JSL::Async::Watcher::Socket clash;
		REQUIRE_NOTHROW(clash.Initialise(socketName, [&](auto msg) {}));
	}

	SECTION("Basic Message Passing")
	{
		std::string output;
		std::mutex Sync;
		std::condition_variable cv;
		JSL::Async::Watcher::Socket Watcher(socketName, [&](auto msg) { output = msg; cv.notify_one(); });

		REQUIRE_NOTHROW(Watcher.Start());
		JSL::Async::Socket::Transmit(socketName, "Hello World");
		std::unique_lock lock(Sync);
		cv.wait(lock, [&] { return !output.empty(); });
		REQUIRE(output == "Hello World");
		REQUIRE_NOTHROW(Watcher.Stop());
	}
	SECTION("Watcher Can Recieve After Restarting")
	{
		std::string output;
		std::mutex Sync;
		std::condition_variable cv;
		JSL::Async::Watcher::Socket Watcher(socketName, [&](auto msg) { output = msg; cv.notify_one(); });

		REQUIRE_NOTHROW(Watcher.Start());
		REQUIRE_NOTHROW(Watcher.Stop());
		REQUIRE_NOTHROW(Watcher.Start());
		JSL::Async::Socket::Transmit(socketName, "Hello World");
		std::unique_lock lock(Sync);
		cv.wait(lock, [&] { return !output.empty(); });
		REQUIRE(output == "Hello World");
		REQUIRE_NOTHROW(Watcher.Stop());
	}
	SECTION("Multiple Messages")
	{

		std::vector<std::string> output;
		std::mutex Sync;
		std::condition_variable cv;
		JSL::Async::Watcher::Socket Watcher(socketName, [&](auto msg) { output.push_back(msg); cv.notify_one(); });

		REQUIRE_NOTHROW(Watcher.Start());
		JSL::Async::Socket::Transmit(socketName, "Hello");
		JSL::Async::Socket::Transmit(socketName, " ");
		JSL::Async::Socket::Transmit(socketName, "World"); // the messages were sent in this order on one thread, so we want to assert that they arrive in this order (this guarantee does not hold if transmissions are themselves asynchronous)
		std::unique_lock lock(Sync);
		cv.wait(lock, [&] { return output.size() == 3; });
		REQUIRE(output.size() == 3);
		REQUIRE(output[0] == "Hello");
		REQUIRE(output[1] == " ");
		REQUIRE(output[2] == "World");
		REQUIRE_NOTHROW(Watcher.Stop());
	}
}

namespace
{
	// A fresh, uniquely-named scratch directory per SECTION run. Catch2
	// re-executes the whole TEST_CASE body for each leaf SECTION, so
	// constructing/destroying this at TEST_CASE scope gives every section
	// a clean directory with no cross-section leftovers to worry about.
	struct TempDir
	{
		std::filesystem::path Path;
		TempDir()
		{
			static std::atomic<int> counter{0};
			auto stamp = std::chrono::steady_clock::now().time_since_epoch().count();
			Path = std::filesystem::temp_directory_path() / ("jsl_watcher_test_" + std::to_string(counter++) + "_" + std::to_string(stamp));
			std::filesystem::create_directories(Path);
		}
		~TempDir()
		{
			std::error_code ec;
			std::filesystem::remove_all(Path, ec);
		}
	};

	// Accumulates every FileChange across every batch the watcher delivers,
	// so a test can wait for a specific change to eventually appear without
	// caring which debounce cycle it arrived in.
	struct BatchCollector
	{
		std::mutex Mutex;
		std::condition_variable Cv;
		std::vector<Watcher::FileChange> AllChanges;

		auto Callback()
		{
			return [this](std::set<Watcher::FileChange> batch) {
				{
					std::lock_guard<std::mutex> lock(Mutex);
					for (auto &change : batch) AllChanges.push_back(change);
				}
				Cv.notify_one();
			};
		}

		// Predicate-based wait with a bounded timeout: safe against both a
		// missed notification (predicate already true = returns
		// immediately) and a hang on regression (times out and returns
		// false, rather than blocking the test run indefinitely).
		bool WaitFor(const std::function<bool(const std::vector<Watcher::FileChange> &)> &pred, std::chrono::milliseconds timeout = std::chrono::seconds(2))
		{
			std::unique_lock<std::mutex> lock(Mutex);
			return Cv.wait_for(lock, timeout, [&] { return pred(AllChanges); });
		}

		bool Contains(const std::string &filename, std::optional<Watcher::ChangeType> change = std::nullopt) const
		{
			return std::any_of(AllChanges.begin(), AllChanges.end(), [&](const Watcher::FileChange &c) {
				return c.Path.filename() == filename && (!change || c.Change == *change);
			});
		}
	};
} // namespace
//
TEST_CASE("File Watcher", "[watcher][file]")
{
	SECTION("Basic Initialisation")
	{
		TempDir dir;
		Watcher::File Watcher;
		REQUIRE_NOTHROW(Watcher.Initialise(dir.Path, true, [](auto) {}));
		REQUIRE_NOTHROW(Watcher.Start());
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("No Spurious Events for Pre-Existing Files on Start")
	{
		// Regression test: the very first snapshot on Start() must be a
		// silent baseline. If it isn't, every pre-existing file in the
		// watched tree fires as "Created" the instant the watcher starts.
		TempDir dir;
		std::ofstream(dir.Path / "preexisting.txt") << "already here";

		BatchCollector collector;
		Watcher::File Watcher(dir.Path, true, collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());

		std::this_thread::sleep_for(std::chrono::milliseconds(300));
		{
			std::lock_guard<std::mutex> lock(collector.Mutex);
			REQUIRE(collector.AllChanges.empty());
		}
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Detects File Creation")
	{
		TempDir dir;
		BatchCollector collector;
		Watcher::File Watcher(dir.Path, true, collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());

		std::ofstream(dir.Path / "hello.txt") << "hi";

		bool found = collector.WaitFor([&](auto &) { return collector.Contains("hello.txt", Watcher::ChangeType::Create); });
		REQUIRE(found);
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Detects File Modification")
	{
		TempDir dir;
		auto filePath = dir.Path / "existing.txt";
		std::ofstream(filePath) << "initial";

		BatchCollector collector;
		Watcher::File Watcher(dir.Path, true, collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());

		// NOTE: modification detection compares mtime+size against the
		// baseline snapshot. A same-tick rewrite on a filesystem with
		// coarse mtime resolution could theoretically be missed -- a short
		// sleep here keeps the test itself independent of that known,
		// accepted limitation rather than exercising it.
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		{
			std::ofstream out(filePath, std::ios::app);
			out << " appended";
		}

		bool found = collector.WaitFor([&](auto &) { return collector.Contains("existing.txt", Watcher::ChangeType::Modify); });
		REQUIRE(found);
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Detects File Deletion")
	{
		TempDir dir;
		auto filePath = dir.Path / "todelete.txt";
		std::ofstream(filePath) << "bye";

		BatchCollector collector;
		Watcher::File Watcher(dir.Path, true, collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		std::filesystem::remove(filePath);

		bool found = collector.WaitFor([&](auto &) { return collector.Contains("todelete.txt", Watcher::ChangeType::Delete); });
		REQUIRE(found);
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Whitelist Filters Non-Matching Files")
	{
		TempDir dir;
		BatchCollector collector;
		Watcher::File Watcher;
		Watcher.Initialise(dir.Path, true, collector.Callback());
		Watcher.AddWhiteList("*.cpp");
		REQUIRE_NOTHROW(Watcher.Start());

		std::ofstream(dir.Path / "match.cpp") << "code";
		std::ofstream(dir.Path / "ignored.txt") << "text";

		bool matchFound = collector.WaitFor([&](auto &) { return collector.Contains("match.cpp"); });
		REQUIRE(matchFound);

		// A little extra settling time to be confident the non-matching
		// file genuinely never arrives, not just "hasn't arrived yet".
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		{
			std::lock_guard<std::mutex> lock(collector.Mutex);
			REQUIRE_FALSE(collector.Contains("ignored.txt"));
		}
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Blacklist Excludes Subdirectory")
	{
		TempDir dir;
		std::filesystem::create_directories(dir.Path / "ignored_dir");

		BatchCollector collector;
		Watcher::File Watcher;
		Watcher.Initialise(dir.Path, true, collector.Callback());
		Watcher.AddBlackList("ignored_dir");
		REQUIRE_NOTHROW(Watcher.Start());

		std::ofstream(dir.Path / "ignored_dir" / "hidden.txt") << "shh";
		std::ofstream(dir.Path / "visible.txt") << "hi";

		bool visibleFound = collector.WaitFor([&](auto &) { return collector.Contains("visible.txt"); });
		REQUIRE(visibleFound);

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		{
			std::lock_guard<std::mutex> lock(collector.Mutex);
			REQUIRE_FALSE(collector.Contains("hidden.txt"));
		}
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Non-Recursive Ignores Subdirectory Changes")
	{
		TempDir dir;
		std::filesystem::create_directories(dir.Path / "subdir");

		BatchCollector collector;
		Watcher::File Watcher(dir.Path, false, collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());

		std::ofstream(dir.Path / "topLevel.txt") << "hi";
		std::ofstream(dir.Path / "subdir" / "nested.txt") << "hi";

		bool topFound = collector.WaitFor([&](auto &) { return collector.Contains("topLevel.txt"); });
		REQUIRE(topFound);

		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		{
			std::lock_guard<std::mutex> lock(collector.Mutex);
			REQUIRE_FALSE(collector.Contains("nested.txt"));
		}
		REQUIRE_NOTHROW(Watcher.Stop());
	}

	SECTION("Can Restart After Stop")
	{
		TempDir dir;
		BatchCollector collector;
		Watcher::File Watcher(dir.Path, true, collector.Callback());
		REQUIRE_NOTHROW(Watcher.Start());
		REQUIRE_NOTHROW(Watcher.Stop());
		REQUIRE_NOTHROW(Watcher.Start());

		std::ofstream(dir.Path / "afterRestart.txt") << "hi";

		bool found = collector.WaitFor([&](auto &) { return collector.Contains("afterRestart.txt"); });
		REQUIRE(found);
		REQUIRE_NOTHROW(Watcher.Stop());
	}
}

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
// TEST_CASE("Manual testing", "[Manual]")
// {
// 	// std::cout << "\n\n>> " << std::flush;
// 	//
// 	// JSL::Async::Watcher::Panopticon AllSeer;
// 	//
// 	// AllSeer.SetInputCallback([](auto line) {
// 	// 	LOG(INFO) << "Transmitting ping to...";
// 	// 	JSL::Async::Socket::Transmit("test.sock", line);
// 	// },
// 	// 	"exit");
// 	//
// 	// AllSeer.SetSocketCallback("test.sock", [&](std::string line) {
// 	// 	LOG(INFO) << "Socket recieved: " << line;
// 	// 	std::cout << ">>" << std::flush;
// 	// });
// 	//
// 	// AllSeer.Start();
// 	//
// 	auto r = JSL::IO::Directory::Snapshot("mantest");
// 	LOG(INFO) << r.ListFiles();
// 	std::mutex R;
// 	std::condition_variable wait;
//
// 	auto W = JSL::Async::Watcher::File("mantest", true, [&](auto batch) {
// 		LOG(INFO) << "New batch: " << batch.size();
// 		for (auto b : batch)
// 		{
// 			LOG(INFO) << b.Path;
// 			if (b.Path.extension() == ".exit")
// 			{
// 				wait.notify_one();
// 			}
// 		}
// 	});
// 	W.Start();
//
// 	std::unique_lock lock(R);
// 	wait.wait(lock);
// }
