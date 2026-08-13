

#include "../test_utils/catch_extended.h"
#include "catch2/matchers/catch_matchers_string.hpp"
#include <JSL.h>
#include <JSL/Async/Socket.h>
#include <JSL/Async/Watcher/FileWatcher.h>
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
	// std::cout << "\n\n>> " << std::flush;
	//
	// JSL::Async::Watcher::Panopticon AllSeer;
	//
	// AllSeer.SetInputCallback([](auto line) {
	// 	LOG(INFO) << "Transmitting ping to...";
	// 	JSL::Async::Socket::Transmit("test.sock", line);
	// },
	// 	"exit");
	//
	// AllSeer.SetSocketCallback("test.sock", [&](std::string line) {
	// 	LOG(INFO) << "Socket recieved: " << line;
	// 	std::cout << ">>" << std::flush;
	// });
	//
	// AllSeer.Start();
	//
	auto r = JSL::IO::Directory::Snapshot("mantest");
	LOG(INFO) << r.ListFiles();
	std::mutex R;
	std::condition_variable wait;

	auto W = JSL::Async::Watcher::File("mantest", true, [&](auto batch) {
		LOG(INFO) << "New batch: " << batch.size();
		for (auto b : batch)
		{
			LOG(INFO) << b.Path;
			if (b.Path.extension() == ".exit")
			{
				wait.notify_one();
			}
		}
	});
	W.Start();

	std::unique_lock lock(R);
	wait.wait(lock);
}
