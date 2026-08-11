#include <JSL/Async/Socket/SocketBase.h>
#include <JSL/internal/error.h>
#include <vector>
namespace JSL::Async::Socket::internal
{

#ifdef WINMODE
	class WinsockContext
	{
	  public:
		static void CheckWinsockActive()
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
	SocketBase::SocketBase()
	{
		WinsockContext::CheckWinsockActive();
	}
#else
	SocketBase::SocketBase() {}
#endif

	const std::vector<char> allowedSymbols = {'-',
		'_',
		'.'};
	void checkName(std::string_view name)
	{
		if (name.empty())
		{
			JSL::internal::LibraryError("Bad identifier", JSL_LOCATION) << "Cannot open a Unix Domain socket with an empty identifier";
		}
		if (name == "." || name == "..")
		{
			JSL::internal::LibraryError("Bad identifier", JSL_LOCATION) << name << " is an invalid socket name";
		}
		for (char c : name)
		{
			if (std::isalnum(c)) continue;

			bool isAllowed = false;
			for (auto allowed : allowedSymbols)
			{
				if (allowed == c)
				{
					isAllowed = true;
					break;
				}
			}

			if (!isAllowed)
			{
				JSL::internal::LibraryError("Bad identifier", JSL_LOCATION) << name << " contains an invalid character (" << c << ")";
			}
		}
	}

	void SocketBase::MakeAddress(std::string_view socketName)
	{
		checkName(socketName);
		// Generate a path in /tmp or /TEMP where the socketfile will live
		SocketPath = std::filesystem::temp_directory_path() / socketName;
		// create the unix socket
		Address = {};
		Address.sun_family = AF_UNIX;

		// validate that we actually have room
		std::string pathstr = SocketPath.string();
		size_t len = pathstr.length();
		if (len < sizeof(Address.sun_path) - 1)
		{
			pathstr.copy(Address.sun_path, len);
		}
		else
		{
			JSL::internal::LibraryError("Bad Identifier", JSL_LOCATION) << pathstr << " too long for a Unix Domain Socket";
		}
	}

} // namespace JSL::Async::Socket::internal
