#include <JSL.h>
#include <JSL/Async/Socket.h>
#include <JSL/Async/Watcher.h>
#include <JSL/Log.h>
#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <vector>
using namespace JSL::Async;
namespace

{
	// A self-cleaning temporary directory
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

	struct FileTester
	{
		std::mutex Mutex;
		std::condition_variable Cv;
		std::atomic<int> CallbackActivated = 0;
		Watcher::File Watcher;
		void StartTest(std::string directory, bool recursive, std::function<void(std::set<Watcher::FileChange>)> callback)
		{
			Initialise(directory, recursive, std::move(callback));
			REQUIRE_NOTHROW(Watcher.Start());
		}
		void StartTest(std::filesystem::path directory, bool recursive, std::function<void(std::set<Watcher::FileChange>)> callback)
		{
			Initialise(directory.string(), recursive, std::move(callback));
			REQUIRE_NOTHROW(Watcher.Start());
		}

		void Initialise(std::string directory, bool recursive, std::function<void(std::set<Watcher::FileChange>)> callback)
		{
			REQUIRE_NOTHROW(
				Watcher.Initialise(directory, recursive, [this, callback = std::move(callback)](auto batch) {
					callback(batch);
					++CallbackActivated;
					Cv.notify_one();
				}));
		}

		void BlockUntilCallback(int milliseconds, int targetActivations = 1)
		{
			std::unique_lock lock(Mutex);
			Cv.wait_for(lock, std::chrono::milliseconds(milliseconds), [&]() { return CallbackActivated >= targetActivations; });
		}
		void EndTest()
		{
			REQUIRE_NOTHROW(Watcher.Stop());
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

		FileTester T;
		T.StartTest(dir.Path, true, [](auto batch) {
			for (auto &obj : batch)
			{
				REQUIRE(obj.Path != "preexisting.txt");
			}
		});
		//
		std::ofstream(dir.Path / "newfile.txt") << "new, triggers callback";
		T.BlockUntilCallback(300);
		REQUIRE(T.CallbackActivated);
		T.EndTest();
	}

	SECTION("Detects File Creation")
	{
		TempDir dir;
		const std::string testname = "hello.txt";

		FileTester T;
		T.StartTest(dir.Path, true, [testname](auto batch) {
			for (auto &obj : batch)
			{
				REQUIRE(obj.Path.filename() == testname);
			}
		});
		std::ofstream(dir.Path / testname) << "hi";
		T.BlockUntilCallback(300);
		REQUIRE(T.CallbackActivated);
		T.EndTest();
	}

	SECTION("Detects File Modification")
	{
		TempDir dir;
		std::string testname = "existing.txt";
		auto filePath = dir.Path / testname;
		std::ofstream(filePath) << "initial";
		FileTester T;
		T.StartTest(dir.Path, true, [&](std::set<Watcher::FileChange> batch) {
			for (const Watcher::FileChange &obj : batch)
			{
				REQUIRE(obj.Path.filename().string() == testname);
				REQUIRE(obj.Object == Watcher::ObjectType::File);
				REQUIRE(obj.Change == Watcher::ChangeType::Modify);
			}
		});

		{
			std::ofstream out(filePath, std::ios::app);
			out << " appended";
		}
		T.BlockUntilCallback(300);
		REQUIRE(T.CallbackActivated);
		T.EndTest();
	}

	SECTION("Detects File Deletion")
	{
		TempDir dir;
		std::string testname = "todelete";
		auto filePath = dir.Path / testname;
		std::ofstream(filePath) << "bye";

		FileTester T;
		T.StartTest(dir.Path, true, [&](std::set<Watcher::FileChange> batch) {
			for (const Watcher::FileChange &obj : batch)
			{
				REQUIRE(obj.Path.filename().string() == testname);
				REQUIRE(obj.Object == Watcher::ObjectType::File);
				REQUIRE(obj.Change == Watcher::ChangeType::Delete);
			}
		});

		std::filesystem::remove(filePath);
		T.BlockUntilCallback(300);
		REQUIRE(T.CallbackActivated);
		T.EndTest();
	}

	SECTION("Whitelist Filters Non-Matching Files")
	{
		TempDir dir;
		FileTester T;
		std::string realfile = "match.cpp";
		T.Initialise(dir.Path, true, [realfile](auto batch) {
			for (const Watcher::FileChange &obj : batch)
			{
				REQUIRE(obj.Path.filename().string() == realfile);
				REQUIRE(obj.Object == Watcher::ObjectType::File);
				REQUIRE(obj.Change == Watcher::ChangeType::Create);
			}
		});

		T.Watcher.AddWhiteList("*.cpp");
		REQUIRE_NOTHROW(T.Watcher.Start());

		std::ofstream(dir.Path / "ignored.txt") << "text"; // create this first so it definitely exists before the real one is created
		std::ofstream(dir.Path / "match.cpp") << "code";

		T.BlockUntilCallback(300, 1); // even though two files created, only 1 of them should trigger the callback
		REQUIRE(T.CallbackActivated);
		T.EndTest();
	}

	SECTION("Blacklist Excludes Subdirectory")
	{
		TempDir dir;
		std::filesystem::create_directories(dir.Path / "ignored_dir");

		FileTester T;

		std::string realfile = "visible.txt";
		T.Initialise(dir.Path, true, [realfile](auto batch) {
			for (const Watcher::FileChange &obj : batch)
			{
				REQUIRE(obj.Path.filename().string() == realfile);
				REQUIRE(obj.Object == Watcher::ObjectType::File);
				REQUIRE(obj.Change == Watcher::ChangeType::Create);
			}
		});
		T.Watcher.AddBlackList("ignored_dir");
		REQUIRE_NOTHROW(T.Watcher.Start());

		std::ofstream(dir.Path / "ignored_dir" / "hidden.txt") << "shh";
		std::ofstream(dir.Path / "visible.txt") << "hi";

		T.BlockUntilCallback(300);
		REQUIRE(T.CallbackActivated);
		T.EndTest();
	}

	SECTION("Non-Recursive Ignores Subdirectory Changes")
	{
		TempDir dir;
		std::filesystem::create_directories(dir.Path / "subdir");

		FileTester T;

		std::string realfile = "topLevel.txt";
		T.StartTest(dir.Path, false, [realfile](auto batch) {
			for (const Watcher::FileChange &obj : batch)
			{
				REQUIRE(obj.Path.filename().string() == realfile);
				REQUIRE(obj.Object == Watcher::ObjectType::File);
				REQUIRE(obj.Change == Watcher::ChangeType::Create);
			}
		});

		std::ofstream(dir.Path / "subdir" / "nested.txt") << "hi";
		std::ofstream(dir.Path / realfile) << "hi";

		T.BlockUntilCallback(300);
		REQUIRE(T.CallbackActivated);
		T.EndTest();
	}

	SECTION("Can Restart After Stop")
	{
		TempDir dir;
		FileTester T;
		std::string realfile = "restart.txt";
		T.StartTest(dir.Path, false, [realfile](auto batch) {
			for (const Watcher::FileChange &obj : batch)
			{
				REQUIRE(obj.Path.filename().string() == realfile);
				REQUIRE(obj.Object == Watcher::ObjectType::File);
				REQUIRE(obj.Change == Watcher::ChangeType::Create);
			}
		});
		REQUIRE_NOTHROW(T.Watcher.Stop());
		REQUIRE_NOTHROW(T.Watcher.Start());

		std::ofstream(dir.Path / realfile) << "hi";

		T.BlockUntilCallback(300);
		REQUIRE(T.CallbackActivated);
		T.EndTest();
	}
}
