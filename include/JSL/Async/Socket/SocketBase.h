#pragma once
#include "CrossPlatformHeaders.h"
#include <filesystem>
namespace JSL::Async::Socket::internal
{
	class SocketBase
	{
	  public:
	  protected:
		std::string Identifier;
		socket_t FileDescriptor = INVALID_SOCKET_VAL;
		sockaddr_un Address = {};
		std::filesystem::path SocketPath;
		void MakeAddress(std::string_view socketName);
	};
} // namespace JSL::Async::Socket::internal
