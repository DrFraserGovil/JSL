#pragma once
#include "SocketBase.h"
#include <filesystem>
#include <string>
#include <string_view>
namespace JSL::Async::Socket
{
	class Broadcaster : public internal::SocketBase
	{
	  public:
		Broadcaster();
		Broadcaster(std::string_view socketName);
		~Broadcaster();

		void SetTarget(std::string_view socketName);
		bool Transmit(std::string_view message);

	  private:
		// Rule of five deletion
		Broadcaster(const Broadcaster &) = delete;
		Broadcaster &operator=(const Broadcaster &) = delete;
		Broadcaster(Broadcaster &&other) noexcept;
		Broadcaster &operator=(Broadcaster &&other) noexcept;
	};
}; // namespace JSL::Async::Socket
