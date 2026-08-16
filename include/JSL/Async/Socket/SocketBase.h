#pragma once
#include "CrossPlatformHeaders.h"
#include <JSL/internal/error.h>
#include <filesystem>
namespace JSL::Async::Socket::internal
{

	//! @brief The base class from which the Listener and Broadcaster derive, providing unified Socket operations
	class SocketBase
	{
	  public:
		//! @brief If necessary, initialise the system sockets library
		//! @details In POSIX platforms, this is a no-op. On Windows, this checks if WSAStartup has been called, and constructs an object that calls WSACleanup() when the program terminates.
		SocketBase();

	  protected:
		//! @brief The name of the socket, used to construct a socketfile.
		std::string Identifier;

		//! @brief The filepath to the socket file.
		//! @details This is set to be $TMP/Identifier, where $TMP is the system-temp directory (if one exists).
		std::filesystem::path SocketPath;

		//! The FileDescriptor associated with the socket
		socket_t FileDescriptor = INVALID_SOCKET_VAL;

		//! The UDS socket address object used to identify the socket object
		sockaddr_un Address = {};

		/*! @brief Performs checks that the provided Identifier is valid, and then usses it to construct the SocketPath and the Address.
			@param socketName The value to be assigned to the Identifier field
			@throws std::runtime_error if the socketName is badly formed (empty, contains non-filesystem characters, too long)
		 */
		void MakeAddress(std::string_view socketName);
	};
} // namespace JSL::Async::Socket::internal
