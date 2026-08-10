#pragma once
#include "SocketBase.h"
#include <chrono>

namespace JSL::Async::Socket
{
	enum class ReadStatus
	{
		Success,
		TimedOut,
		ConnectionClosed,
		MessageTooLong,
		Error
	};

	struct MessageResult
	{
		ReadStatus Status;
		std::string Message;
	};

	class Listener : public internal::SocketBase
	{
	  public:
		Listener();

		Listener(std::string_view socketName, bool forceAcquire = false, std::chrono::milliseconds gracePeriod = std::chrono::milliseconds(50));

		~Listener();
		void Initialise(std::string_view socketName, bool forceAcquire = false, std::chrono::milliseconds gracePeriod = std::chrono::milliseconds(50));

		MessageResult Read(std::chrono::milliseconds timeout = std::chrono::milliseconds(50));
		void Close();

		void SetMaximumPayload(size_t size);

	  private:
		size_t MaxPayload = 4096;
		void QuerySocket(bool forceAcquire, std::chrono::milliseconds gracePeriod);
		bool IsSocketClaimed(bool deleteStale = true);
		void BindSocket();
		std::pair<socket_t, ReadStatus> AcceptIncoming(std::chrono::steady_clock::time_point deadline);
		MessageResult ReadStream(socket_t fd, std::chrono::steady_clock::time_point deadline);
		ReadStatus ExtractFromStream(socket_t fd, char *dest, size_t len, std::chrono::steady_clock::time_point deadline);

		// Rule of five deletion
		Listener(const Listener &) = delete;
		Listener &operator=(const Listener &) = delete;
		Listener(Listener &&other) noexcept;
		Listener &operator=(Listener &&other) noexcept;
	};
}; // namespace JSL::Async::Socket
