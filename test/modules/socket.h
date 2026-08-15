#include <JSL/Async/Socket/Broadcaster.h>
#include <JSL/Async/Socket/Listener.h>
#include <JSL/Async/Socket/Transmit.h>
#include <JSL/Log.h>
#include <catch2/catch_test_macros.hpp>
#include <chrono>
#include <memory>
#include <thread>

using namespace JSL::Async::Socket;

TEST_CASE("Identifier validation", "[socket][validation]")
{
	SECTION("Rejects empty or dot paths")
	{
		REQUIRE_THROWS(Listener(""));
		REQUIRE_THROWS(Listener("."));
		REQUIRE_THROWS(Listener(".."));
	}

	SECTION("Rejects invalid characters")
	{
		REQUIRE_THROWS(Listener("socket/path"));
		REQUIRE_THROWS(Listener("socket name"));
		REQUIRE_THROWS(Listener("socket@name"));
	}

	SECTION("Accepts valid names")
	{
		REQUIRE_NOTHROW(Listener("valid-socket_123.sock"));
	}
}

TEST_CASE("Basic transmission and reception", "[socket][ipc]")
{
	const std::string socketName = "test_basic_ipc.sock";
	Listener listener(socketName, true);
	Broadcaster broadcaster(socketName);

	SECTION("Sends and receives a standard message")
	{
		const std::string payload = "Hello, Unix Domain Sockets!";

		std::thread sender([&broadcaster, &payload]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			REQUIRE(broadcaster.Transmit(payload));
		});

		MessageResult result = listener.Read(std::chrono::milliseconds(500));
		sender.join();

		REQUIRE(result.Status == ReadStatus::Success);
		REQUIRE(result.Message == payload);
	}

	SECTION("Sends and receives an empty message")
	{
		std::thread sender([&broadcaster]() {
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			REQUIRE(broadcaster.Transmit(""));
		});

		MessageResult result = listener.Read(std::chrono::milliseconds(500));
		sender.join();

		REQUIRE(result.Status == ReadStatus::Success);
		REQUIRE(result.Message.empty());
	}

	SECTION("Times out when no data is sent")
	{
		MessageResult result = listener.Read(std::chrono::milliseconds(50));
		REQUIRE(result.Status == ReadStatus::TimedOut);
		REQUIRE(result.Message.empty());
	}
}

TEST_CASE("Sequential message passing", "[socket][ipc]")
{
	const std::string socketName = "test_sequential_ipc.sock";
	Listener listener(socketName, true);
	Broadcaster broadcaster(socketName);

	for (int i = 0; i < 5; ++i)
	{
		std::string payload = "Message iteration " + std::to_string(i);
		REQUIRE(broadcaster.Transmit(payload));

		MessageResult result = listener.Read(std::chrono::milliseconds(100));
		REQUIRE(result.Status == ReadStatus::Success);
		REQUIRE(result.Message == payload);
	}
}

TEST_CASE("Broadcaster error handling", "[socket][broadcaster]")
{
	SECTION("Fails gracefully when target is unconfigured")
	{
		Broadcaster broadcaster;
		REQUIRE_THROWS(broadcaster.Transmit("lost message"));
	}

	SECTION("Fails gracefully when listener does not exist")
	{
		Broadcaster broadcaster("non_existent_socket.sock");
		REQUIRE_THROWS(broadcaster.Transmit("lost message"));
	}
}

TEST_CASE("Listener hostile takeover", "[socket][takeover]")
{
	const std::string socketName = "test_takeover.sock";
	auto listener1 = std::make_unique<Listener>(socketName, true);

	SECTION("Fails to claim occupied socket without forceAcquire")
	{
		REQUIRE_THROWS(Listener(socketName, false));
	}

	SECTION("Takes over socket when forceAcquire is true")
	{
		std::thread receiver([&listener1]() {
			while (true)
			{
				MessageResult res = listener1->Read(std::chrono::milliseconds(2000));
				if (res.Status == ReadStatus::Success && res.Message == "exit")
				{
					listener1->Close();
					break;
				}
				// Retry if the socket consumed a transient probe connection
				if (res.Status != ReadStatus::ConnectionClosed)
				{
					break;
				}
			}
		});

		// Transmit(socketName, "exit");

		REQUIRE_NOTHROW(Listener(socketName, true, std::chrono::milliseconds(100)));
		receiver.join();
	}
}

TEST_CASE("Listener rejects payloads that are too long", "[socket][listener]")
{
	const std::string socketName = "test_takeover.sock";
	auto listener1 = std::make_unique<Listener>(socketName, true);
	size_t maxSize = 100;
	listener1->SetMaximumPayload(maxSize);
	SECTION("Throws an error")
	{
		std::string bigMessage(maxSize + 1, 'a');
		std::string smallMessage(maxSize - 1, 'b');
		Broadcaster B(socketName);

		B.Transmit(bigMessage);
		REQUIRE(listener1->Read().Status == ReadStatus::MessageTooLong);
		B.Transmit(smallMessage);
		REQUIRE(listener1->Read().Status == ReadStatus::Success);
	}
}
