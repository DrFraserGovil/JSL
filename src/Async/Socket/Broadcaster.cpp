#include <JSL/Async/Socket/Broadcaster.h>
#include <JSL/Log.h>
#include <JSL/internal/error.h>
namespace fs = std::filesystem;

namespace JSL::Async::Socket
{
	Broadcaster::Broadcaster() : internal::SocketBase()
	{
	}

	Broadcaster::Broadcaster(std::string_view socketName) : internal::SocketBase()
	{
		SetTarget(socketName);
	}

	void Broadcaster::SetTarget(std::string_view socketName)
	{
		Identifier = static_cast<std::string>(socketName);
		MakeAddress(socketName);
	}

	bool Broadcaster::Transmit(std::string_view message)
	{
		if (Identifier.empty())
		{
			JSL::internal::LibraryError("Invalid target", JSL_LOCATION) << "Cannot transmit without a target socket name";
		}

		std::error_code ec;
		if (fs::symlink_status(SocketPath, ec).type() == fs::file_type::not_found)
		{
			JSL::internal::LibraryError("No listener", JSL_LOCATION) << "There is no socketfile established at " << SocketPath.string() << ". Create an Async::Listener";
		}
		int fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd == INVALID_SOCKET_VAL)
		{
			return false;
		}

		if (connect(fd, (struct sockaddr *)&Address, sizeof(Address)) != 0)
		{
			CLOSE(fd);
			return false;
		}

		uint32_t len = static_cast<uint32_t>(message.length());
		if (send(fd, reinterpret_cast<const char *>(&len), sizeof(len), 0) != sizeof(len))
		{
			CLOSE(fd);

			return false;
		}

		if (len > 0)
		{
			size_t sent = 0;
			while (sent < len)
			{
				auto n = send(fd, message.data() + sent, len - sent, 0);
				if (n < 0)
				{
					if (errno == EINTR) continue;
					CLOSE(fd);
					return false;
				}
				sent += static_cast<size_t>(n);
			}
		}

		CLOSE(fd);
		return true;
	}
} // namespace JSL::Async::Socket
