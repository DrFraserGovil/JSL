

#include "../test_utils/catch_extended.h"
#include "catch2/matchers/catch_matchers_string.hpp"
#include <JSL.h>
#include <JSL/Async/Socket.h>
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
	auto dir = JSL::IO::Directory::Snapshot("./", ".build");
	LOG(INFO) << dir.ListAll();
}
