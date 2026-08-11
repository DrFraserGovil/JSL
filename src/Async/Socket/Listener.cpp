#include "JSL/Async/Socket/Broadcaster.h"
#include <JSL/Async/Socket/Listener.h>
#include <JSL/Async/Socket/Transmit.h>
#include <JSL/Log.h>
#include <JSL/internal/error.h>
#include <thread>
namespace fs = std::filesystem;
namespace JSL::Async::Socket
{
	Listener::Listener()
	{
	}

	Listener::Listener(std::string_view socketName, bool forceAcquire, std::chrono::milliseconds gracePeriod) : Listener()
	{
		Initialise(socketName, forceAcquire, gracePeriod);
	}

	void Listener::Initialise(std::string_view socketName, bool forceAcquire, std::chrono::milliseconds gracePeriod)
	{
		// check if we have an open connection, if so, terminate it
		if (FileDescriptor != INVALID_SOCKET_VAL)
		{
			Close();
		}

		// save this for later diagnostic use
		Identifier = static_cast<std::string>(socketName);

		MakeAddress(socketName);
		QuerySocket(forceAcquire, gracePeriod);
		BindSocket();
	}
	MessageResult Listener::Read(std::chrono::milliseconds timeout)
	{
		if (FileDescriptor == INVALID_SOCKET_VAL)
		{
			JSL::internal::LibraryError("Invalid socket", JSL_LOCATION) << "Cannot read from an uninitialised or expired socket";
		}
		auto deadline = std::chrono::steady_clock::now() + timeout;

		auto [clientFd, status] = AcceptIncoming(deadline);
		if (status != ReadStatus::Success)
		{
			return {status, ""};
		}

		MessageResult result = ReadStream(clientFd, deadline);
		CLOSE(clientFd);
		return result;
	}

	void Listener::QuerySocket(bool forceAcquire, std::chrono::milliseconds gracePeriod)
	{

		bool socketClaimed = IsSocketClaimed();

		if (!socketClaimed) return;

		// if we're here, then there is a non-stale socket file

		if (!forceAcquire)
		{
			// if we're not feeling greedy, then this is a fail-state
			JSL::internal::LibraryError("Socket Occupied", JSL_LOCATION) << SocketPath.string() << " is already being monitored by another process; cannot establish a listener-connection";
		}

		// if we are greedy, then begin the takeover process
		LOG(DEBUG) << "Attempting a hostile takeover of the other active process";

		bool messageSent = Transmit(Identifier, "exit");
		if (!messageSent)
		{
			JSL::internal::LibraryError("Socket corrupted", JSL_LOCATION) << "A process is occupying " << SocketPath.string() << " but is blocking incoming messages";
		}
		std::this_thread::sleep_for(gracePeriod);
		bool stillClaimed = IsSocketClaimed();
		if (stillClaimed)
		{
			JSL::internal::LibraryError("Socket Occupied", JSL_LOCATION) << "A process has claimed " << SocketPath.string() << " and is refusing to release it";
		}
	}

	bool Listener::IsSocketClaimed(bool deleteStale)
	{
		std::error_code ec;
		if (fs::status(SocketPath, ec).type() == fs::file_type::not_found)
		{
			return false;
		}

		int fd = socket(AF_UNIX, SOCK_STREAM, 0);
		if (fd == INVALID_SOCKET_VAL)
		{
			JSL::internal::LibraryError("Bad FileDescriptor", JSL_LOCATION) << "socket() threw an error when creating " << Identifier;
		}

		bool connected = (connect(fd, (struct sockaddr *)&Address, sizeof(Address)) == 0);

		bool nobodyListening = !connected && CONNECTION_REFUSED;

		CLOSE(fd); // fd is a temp file descriptor, so close it out

		if (nobodyListening)
		{
			LOG(DEBUG) << "Stale socket file taken over by this process";
			if (deleteStale) fs::remove(SocketPath);
			return false; // stale: nobody's actually listening
		}
		return true;
	}

	void Listener::BindSocket()
	{
		int fd = socket(AF_UNIX, SOCK_STREAM, 0);

		if (fd == INVALID_SOCKET_VAL)
		{
			JSL::internal::LibraryError("Failed to bind socket", JSL_LOCATION) << "Failed to create a socket at " << SocketPath.string();
		}
		if (bind(fd, (struct sockaddr *)&Address, sizeof(Address)) != 0)
		{
			CLOSE(fd); // fd is now a valid descriptor, so need to remember to close it
			JSL::internal::LibraryError("Failed to bind socket", JSL_LOCATION) << "Failed to bind a socket to " << SocketPath.string();
		}
		if (listen(fd, SOMAXCONN) != 0)
		{
			CLOSE(fd);
			fs::remove(SocketPath); // bind succeeded so now need to clean this up too
			JSL::internal::LibraryError("Failed to bind socket", JSL_LOCATION) << "Failed to listen to the socket at " << SocketPath.string();
		}
		LOG(DEBUG) << "Listener successfully bound to " << SocketPath.string();
		FileDescriptor = fd;
	}

	timeval ToTimeval(std::chrono::steady_clock::duration duration)
	{
		timeval t;
		auto millisecs = std::chrono::duration_cast<std::chrono::milliseconds>(duration);

		t.tv_sec = millisecs.count() / 1000;
		t.tv_usec = (millisecs.count() % 1000) * 1000;
		return t;
	}

	std::pair<socket_t, ReadStatus> Listener::AcceptIncoming(std::chrono::steady_clock::time_point deadline)
	{
		while (true)
		{
			auto remaining = deadline - std::chrono::steady_clock::now();
			if (remaining <= std::chrono::steady_clock::duration::zero())
			{
				return {INVALID_SOCKET_VAL, ReadStatus::TimedOut}; // timed out
			}

			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(FileDescriptor, &readSet);

			timeval tv = ToTimeval(remaining);
			int ready = select(static_cast<int>(FileDescriptor) + 1, &readSet, nullptr, nullptr, &tv);

			if (ready == 0)
			{
				return {INVALID_SOCKET_VAL, ReadStatus::TimedOut}; // timed out
			}
			if (ready < 0)
			{
				if (errno == EINTR) continue;					// interrupted, just retry with the same deadline
				return {INVALID_SOCKET_VAL, ReadStatus::Error}; // genuine error state
			}

			socket_t fd = accept(FileDescriptor, nullptr, nullptr);
			if (fd == INVALID_SOCKET_VAL)
			{
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;

				return {INVALID_SOCKET_VAL, ReadStatus::Error};
			}
			return {fd, ReadStatus::Success};
		}
	}

	MessageResult Listener::ReadStream(socket_t fd, std::chrono::steady_clock::time_point deadline)
	{

		// first must read the length of the message -- so read sizeof(uint32_t) bytes from the stream, which we set by convention to be the length of the next incoming message
		uint32_t len = 0;
		auto messageLengthStatus = ExtractFromStream(fd, reinterpret_cast<char *>(&len), sizeof(len), deadline);
		if (messageLengthStatus != ReadStatus::Success)
		{
			return {messageLengthStatus, ""};
		}

		if (len > MaxPayload)
		{
			return {ReadStatus::MessageTooLong, ""};
		}

		// now we know the length, we can read in the message
		std::string msg(len, '\0');
		if (len > 0)
		{
			auto fullMsgStatus = ExtractFromStream(fd, msg.data(), len, deadline);
			if (fullMsgStatus != ReadStatus::Success)
			{
				return {fullMsgStatus, ""};
			}
		}

		return {ReadStatus::Success, std::move(msg)};
	}

	ReadStatus Listener::ExtractFromStream(socket_t fd, char *dest, size_t len, std::chrono::steady_clock::time_point deadline)
	{
		size_t received = 0;
		while (received < len)
		{
			auto remaining = deadline - std::chrono::steady_clock::now();
			if (remaining <= std::chrono::steady_clock::duration::zero())
			{
				return ReadStatus::TimedOut; // timed out mid-message
			}

			fd_set readSet;
			FD_ZERO(&readSet);
			FD_SET(fd, &readSet);
			timeval tv = ToTimeval(remaining);

			int ready = select(static_cast<int>(fd) + 1, &readSet, nullptr, nullptr, &tv);
			if (ready == 0) return ReadStatus::TimedOut; // timed out
			if (ready < 0)
			{
				if (errno == EINTR) continue;

				return ReadStatus::Error;
			}

			auto n = recv(fd, dest + received, len - received, 0);
			if (n == 0)
			{
				return ReadStatus::ConnectionClosed; // peer closed mid-message: treat as ConnectionClosed at the caller
			}
			if (n < 0)
			{
				if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK) continue;
				return ReadStatus::Error;
			}
			received += static_cast<size_t>(n);
		}
		return ReadStatus::Success;
	}

	void Listener::Close()
	{
		LOG(DEBUG) << "Closing listener connection to " << SocketPath.string();
		CLOSE(FileDescriptor);
		fs::remove(SocketPath);
		FileDescriptor = INVALID_SOCKET_VAL;
	}
	void Listener::SetMaximumPayload(size_t size)
	{
		MaxPayload = size;
	}

	Listener::~Listener()
	{
		if (FileDescriptor != INVALID_SOCKET_VAL)
		{
			Close();
		}
	}
} // namespace JSL::Async::Socket
