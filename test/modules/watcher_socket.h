#include <JSL.h>
#include <JSL/Async/Socket.h>
#include <JSL/Async/Watcher.h>
#include <JSL/Log.h>
#include <catch2/catch_test_macros.hpp>
#include <condition_variable>
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
