

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
#include <vector>
using namespace JSL::Async;
using ms = std::chrono::milliseconds;
#include "watcher_file.h"
#include "watcher_input.h"
#include "watcher_socket.h"

namespace
{
	// Rather than running the Panopticon on a separate thread, we run the stimulus on a thread, repeatedly pinging the signal
	struct Stimulator
	{
		std::atomic<bool> &Done;
		std::thread Thread;
		std::exception_ptr Error{};
		bool Timeout = false;
		template <class Fn>
		Stimulator(std::atomic<bool> &done, Watcher::Panopticon &panopticon, Fn stimulus, ms interval = ms(20), ms timeout = std::chrono::seconds(1)) : Done(done)
		{
			auto deadline = std::chrono::steady_clock::now() + timeout;
			std::atomic<int> count = 0;
			Thread = std::thread([this, &panopticon, stimulus, interval, &deadline, &count] {
				while (!Done.load() && std::chrono::steady_clock::now() < deadline)
				{
					try
					{
						stimulus();
					}
					catch (...)
					{
						Error = std::current_exception();
						Done = true;
						panopticon.Stop();
						return;
					}
					++count;
					std::this_thread::sleep_for(interval);
				}
				if (!Done.load())
				{
					Timeout = true;
					panopticon.Stop(); // watchdog: unblock Start() so the test fails, rather than hangs
				}
			});
			panopticon.Start();

			Done = true; // prevents stim hanging if Start() stops blocking
			Thread.join();
			if (Error)
			{
				std::rethrow_exception(Error);
			}
		}
		~Stimulator()
		{
			if (Thread.joinable()) Thread.join();
		}
	};
} // namespace

TEST_CASE("Panopticon", "[watcher][panopticon]")
{
	SECTION("Throws error if start called on a blank object")
	{
		Watcher::Panopticon panopticon;
		REQUIRE_THROWS(panopticon.Start());
	}

	SECTION("Dispatches Input Callback")
	{
		StdinSplicer splicer;
		std::atomic<bool> noTimeout{false};
		Watcher::Panopticon panopticon;
		std::string expectedMessage = "hello panopticon";
		std::string recievedMessage;
		panopticon.SetInputCallback([&](auto msg) {
			recievedMessage = msg;
			noTimeout = true;
			panopticon.Stop();
		});

		Stimulator stim(noTimeout, panopticon, [&] {
			REQUIRE_NOTHROW(splicer.Write(expectedMessage + "\n"));
		});

		REQUIRE(noTimeout);
		REQUIRE(recievedMessage == expectedMessage);
	}
	//
	SECTION("Exit String Stops the Panopticon Without an Explicit Stop")
	{
		StdinSplicer splicer;
		std::atomic<bool> gotCallback{false};
		Watcher::Panopticon panopticon;
		std::string recievedMessage;
		panopticon.SetInputCallback([&](auto msg) {
			recievedMessage = msg;
			gotCallback = true;
			panopticon.Stop();
		},
			"exit");

		Stimulator stim(gotCallback, panopticon, [&] {
			REQUIRE_NOTHROW(splicer.Write("exit\n"));
		});

		REQUIRE(recievedMessage.empty());
		REQUIRE(!stim.Timeout);
	}

	SECTION("Dispatches Socket Callback")
	{
		std::string socketName = "panopticon_test.sock";
		std::atomic<bool> noTimeout{false};
		Watcher::Panopticon panopticon;
		std::string expectedMessage = "hello panopticon";
		std::string recievedMessage;
		panopticon.SetSocketCallback(socketName, [&](auto msg) {
			recievedMessage = msg;
			noTimeout = true;
			panopticon.Stop();
		});

		Stimulator stim(noTimeout, panopticon, [&] {
			JSL::Async::Socket::Transmit(socketName, expectedMessage);
		});

		REQUIRE(noTimeout);
		REQUIRE(recievedMessage == expectedMessage);
	}
	//
	SECTION("Dispatches File Batch Callback")
	{
		TempDir dir;
		Watcher::Panopticon panopticon;
		panopticon.SetDebounceTime(50);
		std::string name = "monitored.txt";
		std::ofstream(dir.Path / name) << "hi";
		std::string gotname;
		std::atomic<bool> noTimeout{false};
		panopticon.SetFileBatchCallback(dir.Path.string(), true, [&](auto batch) {
			for (const Watcher::FileChange &file : batch)
			{
				gotname = file.Path.filename().string();
				REQUIRE(file.Change == Watcher::ChangeType::Modify);
				REQUIRE(file.Object == Watcher::ObjectType::File);
			}
			noTimeout = true;
			panopticon.Stop();
		});

		Stimulator stim(noTimeout, panopticon, [&] {
			{
				std::ofstream out(dir.Path / name, std::ios::app);
				out << " appended";
			}
		});

		REQUIRE(noTimeout);
		REQUIRE(gotname == name);
	}

	//
	SECTION("Refuses to Overwrite an Existing Callback by Default")
	{
		std::string socketName = "panopticon_overwrite_test.sock";
		Watcher::Panopticon panopticon;
		REQUIRE_NOTHROW(panopticon.SetSocketCallback(socketName, [](auto) {}));
		REQUIRE_THROWS(panopticon.SetSocketCallback(socketName, [](auto) {}));
		REQUIRE_NOTHROW(panopticon.SetSocketCallback(socketName, [](auto) {}, false, true));
	}
	//
	SECTION("Refuses to Register Callbacks While Running")
	{
		Watcher::Panopticon panopticon;
		TempDir dir;

		panopticon.SetInputCallback([](auto) {});
		panopticon.SetSocketCallback("test.sock", [](auto) {});
		panopticon.SetFileBatchCallback(dir.Path, true, [](auto) {});

		std::atomic<bool> blank{false};
		REQUIRE_THROWS(
			Stimulator(blank, panopticon, [&] {
				panopticon.SetInputCallback([](auto) {});
			}));
		blank = false;
		REQUIRE_THROWS(
			Stimulator(blank, panopticon, [&] {
				panopticon.SetSocketCallback("test.sock", [](auto) {});
			}));
		blank = false;
		REQUIRE_THROWS(
			Stimulator(blank, panopticon, [&] {
				panopticon.SetFileBatchCallback(dir.Path, true, [](auto) {});
			}));
	}
}
