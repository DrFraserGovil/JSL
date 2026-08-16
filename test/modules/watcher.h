

#include "../test_utils/catch_extended.h"
#include "catch2/matchers/catch_matchers_string.hpp"
#include <JSL.h>
#include <JSL/Async/Socket.h>
#include <JSL/Async/Watcher.h>
#include <JSL/Log.h>
#include <catch2/catch_test_macros.hpp>
#include <future>

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
		bool WaitFor(const std::function<bool(const std::vector<Watcher::FileChange> &)> &pred, std::chrono::milliseconds timeout = std::chrono::seconds(5))
		{
			std::unique_lock<std::mutex> lock(Mutex);
			Cv.wait_for(lock, timeout, [&] { return pred(AllChanges); });
			return true;
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

namespace
{

	// Panopticon::Start() blocks until Stop() (or an exitString match) is
	// hit, so every test needs to run it on its own thread. Returns a
	// future so tests can assert the thread actually terminated within a
	// bounded time, rather than joining unconditionally and risking a
	// test-suite hang on regression.
	std::future<void> RunAsync(Watcher::Panopticon &panopticon)
	{
		return std::async(std::launch::async, [&panopticon] { panopticon.Start(); });
	}

	bool FinishesWithin(std::future<void> &future, std::chrono::milliseconds timeout = std::chrono::seconds(2))
	{
		return future.wait_for(timeout) == std::future_status::ready;
	}

	// A failing REQUIRE throws and unwinds out of the SECTION immediately,
	// skipping any explicit panopticon.Stop() call written further down.
	// Left unstopped, Start() never returns -- and a std::future from
	// std::async blocks in its destructor until the async task actually
	// finishes. Without this guard, a single failed assertion turns into
	// a hung test binary instead of a clean failure. Declare this AFTER
	// the future returned by RunAsync(), so it destructs (and calls
	// Stop()) BEFORE that future does, on every exit path.
	struct AutoStop
	{
		Watcher::Panopticon &Target;
		~AutoStop()
		{
			Target.Stop();
		}
	};
} // namespace
  //
  //

TEST_CASE("Panopticon", "[watcher][panopticon]")
{
	SECTION("Starts and Stops With Nothing Registered")
	{
		Watcher::Panopticon panopticon;
		auto result = RunAsync(panopticon);
		AutoStop stopper{panopticon}; // guarantees Stop() runs even if a REQUIRE below throws
		panopticon.Stop();
		REQUIRE(FinishesWithin(result));
		REQUIRE_NOTHROW(result.get());
	}

	SECTION("Dispatches Input Callback")
	{
		StdinSplicer splicer;
		LineCollector collector;
		Watcher::Panopticon panopticon;
		panopticon.SetInputCallback(collector.Callback());

		auto result = RunAsync(panopticon);
		AutoStop stopper{panopticon}; // guarantees Stop() runs even if a REQUIRE below throws
		splicer.Write("hello panopticon\n");

		bool found = collector.WaitFor([](auto &lines) { return !lines.empty(); });
		REQUIRE(found);
		REQUIRE(collector.Lines[0] == "hello panopticon");

		panopticon.Stop();
		REQUIRE(FinishesWithin(result));
	}

	SECTION("Exit String Stops the Panopticon Without an Explicit Stop")
	{
		StdinSplicer splicer;
		LineCollector collector;
		Watcher::Panopticon panopticon;
		panopticon.SetInputCallback(collector.Callback(), "exit");

		auto result = RunAsync(panopticon);
		AutoStop stopper{panopticon}; // guarantees Stop() runs even if a REQUIRE below throws
		splicer.Write("exit\n");

		// No explicit Stop() call here -- the exitString match should be
		// enough on its own to unblock Start().
		REQUIRE(FinishesWithin(result));
		REQUIRE_NOTHROW(result.get());
	}

	SECTION("Dispatches Socket Callback")
	{
		std::string socketName = "panopticon_test.sock";
		LineCollector collector;
		Watcher::Panopticon panopticon;
		panopticon.SetSocketCallback(socketName, collector.Callback());

		auto result = RunAsync(panopticon);
		AutoStop stopper{panopticon}; // guarantees Stop() runs even if a REQUIRE below throws
		// Give the socket a moment to actually bind before transmitting.
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		Socket::Transmit(socketName, "hello socket");

		bool found = collector.WaitFor([](auto &lines) { return !lines.empty(); });
		REQUIRE(found);
		REQUIRE(collector.Lines[0] == "hello socket");

		panopticon.Stop();
		REQUIRE(FinishesWithin(result));
	}

	SECTION("Dispatches File Batch Callback")
	{
		TempDir dir;
		BatchCollector collector;
		Watcher::Panopticon panopticon;
		panopticon.SetFileBatchCallback(dir.Path.string(), true, collector.Callback());

		auto result = RunAsync(panopticon);
		AutoStop stopper{panopticon}; // guarantees Stop() runs even if a REQUIRE below throws
		// Give the watcher a moment to establish its initial baseline
		// snapshot before creating the file -- otherwise the file could
		// land inside that baseline and never fire as Create at all, per
		// the "No Spurious Events for Pre-Existing Files" behavior.
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		std::ofstream(dir.Path / "created.txt") << "hi";

		bool found = collector.WaitFor([&](auto &) { return collector.Contains("created.txt", Watcher::ChangeType::Create); });
		REQUIRE(found);

		panopticon.Stop();
		REQUIRE(FinishesWithin(result));
	}

	SECTION("Dispatches Single File Callback Per-File")
	{
		TempDir dir;
		std::mutex mutex;
		std::condition_variable cv;
		std::vector<Watcher::FileChange> received;

		Watcher::Panopticon panopticon;
		panopticon.SetSingleFileCallback(dir.Path.string(), true, [&](Watcher::FileChange change) {
			{
				std::lock_guard<std::mutex> lock(mutex);
				received.push_back(change);
			}
			cv.notify_one();
		});

		auto result = RunAsync(panopticon);
		AutoStop stopper{panopticon}; // guarantees Stop() runs even if a REQUIRE below throws
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
		std::ofstream(dir.Path / "a.txt") << "a";
		std::ofstream(dir.Path / "b.txt") << "b";

		std::unique_lock<std::mutex> lock(mutex);
		bool found = cv.wait_for(lock, std::chrono::seconds(2), [&] { return received.size() >= 2; });
		REQUIRE(found);
		REQUIRE(std::any_of(received.begin(), received.end(), [](auto &c) { return c.Path.filename() == "a.txt"; }));
		REQUIRE(std::any_of(received.begin(), received.end(), [](auto &c) { return c.Path.filename() == "b.txt"; }));
		lock.unlock();

		panopticon.Stop();
		REQUIRE(FinishesWithin(result));
	}

	SECTION("Refuses to Overwrite an Existing Callback by Default")
	{
		std::string socketName = "panopticon_overwrite_test.sock";
		Watcher::Panopticon panopticon;
		REQUIRE_NOTHROW(panopticon.SetSocketCallback(socketName, [](auto) {}));
		REQUIRE_THROWS(panopticon.SetSocketCallback(socketName, [](auto) {}));
		REQUIRE_NOTHROW(panopticon.SetSocketCallback(socketName, [](auto) {}, false, true));
	}

	SECTION("Refuses to Register Callbacks While Running")
	{
		Watcher::Panopticon panopticon;
		auto result = RunAsync(panopticon);
		AutoStop stopper{panopticon}; // guarantees Stop() runs even if a REQUIRE below throws
		// give Start() a moment to actually flip IsRunning before we probe it
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		REQUIRE_THROWS(panopticon.SetInputCallback([](auto) {}));
		REQUIRE_THROWS(panopticon.SetSocketCallback("wont_bind.sock", [](auto) {}));

		panopticon.Stop();
		REQUIRE(FinishesWithin(result));
	}

	SECTION("Refuses a Second Start While Already Running")
	{
		Watcher::Panopticon panopticon;
		auto result = RunAsync(panopticon);
		AutoStop stopper{panopticon}; // guarantees Stop() runs even if a REQUIRE below throws
		std::this_thread::sleep_for(std::chrono::milliseconds(50));

		REQUIRE_THROWS(panopticon.Start());

		panopticon.Stop();
		REQUIRE(FinishesWithin(result));
	}
}
