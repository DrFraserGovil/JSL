

#include "../test_utils/catch_extended.h"
#include "catch2/matchers/catch_matchers_string.hpp"
#include <JSL/Async/Watcher/Panopticon.h>
#include <JSL/Log.h>
#include <catch2/catch_test_macros.hpp>

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <numeric>
#include <vector>
using namespace JSL::Async;

TEST_CASE("Manual testing", "[Manual]")
{
	std::cout << ">> " << std::flush;

	JSL::Async::Watcher::Panopticon AllSeer;

	AllSeer.SetInputCallback([](auto line) {
		LOG(WARN) << "Got your message: " << line;
	},
		"exit");

	AllSeer.Start();
	// std::mutex Pause;
	//
	//
	//
	// std::unique_lock lock(Pause);
	// std::condition_variable Lock;
	// JSL::Async::Watcher::Input W(
	// 	[&](auto line) {
	// 		LOG(WARN) << "User gave: " << line;
	// 		std::cout << ">> " << std::flush;
	//
	// 		if (line == "exit")
	// 		{
	// 			Lock.notify_one();
	// 		}
	// 	});
	// W.Start();
	//
	// Lock.wait(lock);
	// W.Stop();
}
