#pragma once
#include "CrossPlatformHeaders.h"
#include <filesystem>
namespace JSL::Async::Socket::internal
{
#ifdef WINMODE
	class WinsockContext
	{
	  public:
		static void Ensure()
		{
			static WinsockContext instance; // constructed exactly once, thread-safe per C++11 static-local rules
			(void)instance;
		}

	  private:
		WinsockContext()
		{
			WSADATA data;
			int result = WSAStartup(MAKEWORD(2, 2), &data);
			if (result != 0)
			{
				JSL::internal::LibraryError("Failed to initialise Winsock", JSL_LOCATION)
					<< "WSAStartup failed with error " << result;
			}
		}
		~WinsockContext()
		{
			WSACleanup();
		}
		WinsockContext(const WinsockContext &) = delete;
		WinsockContext &operator=(const WinsockContext &) = delete;
	};
#else
	class WinsockContext
	{
	  public:
		static void Ensure() {} // nothing to do on POSIX
	};
#endif
	class SocketBase
	{
	  public:
		SocketBase() { WinsockContext::Ensure(); }

	  protected:
		std::string Identifier;
		socket_t FileDescriptor = INVALID_SOCKET_VAL;
		sockaddr_un Address = {};
		std::filesystem::path SocketPath;
		void MakeAddress(std::string_view socketName);
	};
} // namespace JSL::Async::Socket::internal
