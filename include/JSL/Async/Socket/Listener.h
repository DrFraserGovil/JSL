#pragma once
#include "SocketBase.h"
#include <chrono>

namespace JSL::Async::Socket
{
	//! @brief An enum used to describe the status of a returned message from the Listener
	enum class ReadStatus
	{
		//! @brief The message is good
		Success,
		//! @brief The read() call timed out before the message was recieved. There is no message to read.
		TimedOut,
		//! @brief The socket is dead. There is no message to read.
		ConnectionClosed,
		//! @brief The Broadcaster indicated a message which exceeded the buffer size. There is no message to read.
		MessageTooLong,
		//! @brief Something else went wrong. There is no message to read.
		Error
	};

	//! @brief Holds the result of a Read() call to the Listener
	struct MessageResult
	{
		//! @brief If this is ReadStatus::Success, then the message can be read, otherwise Message will be blank.
		ReadStatus Status;
		//! @brief The string transmitted to the socket
		std::string Message;
	};

	class Listener : public internal::SocketBase
	{
	  public:
		//! @brief Just to avoid really long function signatures
		using milliseconds = std::chrono::milliseconds;
		//! @brief A blank constructor that places the object in an uninitialised state
		Listener();

		/*! @brief A constructor which initialises the object (equivalent to a = Listener(); a.Initialise(..)) After this is called, no other sockets with the same identifier can be created until Close() is called.
			@param socketName The Identifier to be assigned to the Socket (and from which the socketfile name will be derived)
			@param forceAcquire If false and a socketfile already exists, the constructor will throw. If true, the object will attempt a takeover of the socket file, signalling the existing object to release the socket.
			@param gracePeriod The time given for the forceAcquire routine to work. If the socket is not released during this time period, the constructor will throw an error
			@throws std::runtime_error If the socketName is invlaid (per SocketBase::MakeAddress)
			@throws std::runtime_error If the socketfile already exists, and forceAcquire is false
			@throws std::runtime_error If forceAcquire is true, but the gracePeriod elapses without the socketfile being released by the other process
			@throws std::runtime_error If the system calls for creating or binding the socket return an error
		 */
		Listener(std::string_view socketName, bool forceAcquire = false, milliseconds gracePeriod = milliseconds(50));

		//! @brief If the socket remains open, calls Close() when passing out of scope
		~Listener();

		/*!	@brief (Re)initialise a socket with the given identifier. After this is called, no other sockets with the same identifier can be created until Close() is called.
			@details If the socket was previously initialised, Close() is called automatically to release the resources
			@param socketName The Identifier to be assigned to the Socket (and from which the socketfile name will be derived)
			@param forceAcquire If false and a socketfile already exists, the constructor will throw. If true, the object will attempt a takeover of the socket file, signalling the existing object to release the socket.
			@param gracePeriod The time given for the forceAcquire routine to work. If the socket is not released during this time period, the constructor will throw an error
			@throws std::runtime_error If the socketName is invlaid (per SocketBase::MakeAddress)
			@throws std::runtime_error If the socketfile already exists, and forceAcquire is false
			@throws std::runtime_error If forceAcquire is true, but the gracePeriod elapses without the socketfile being released by the other process
			@throws std::runtime_error If the system calls for creating or binding the socket return an error
		 */
		void Initialise(std::string_view socketName, bool forceAcquire = false, milliseconds gracePeriod = milliseconds(50));

		/*! @brief A blocking call which waits for a message to arrive
			@param timeout The time (in milliseconds) to block on this thread, before returning a ReadStatus::Timeout
			@returns An object which contains either an error-code, or the message given to the socket
			@throws std::runtime_error If the object was not initialised before Read() was called
		 */
		MessageResult Read(milliseconds timeout = milliseconds(50));

		//! @brief Closes the file descriptors and removes the socketfile, releasing it for the next user
		void Close();

		//! Sets the number of bytes (~= number of characters) a message can contain before a ReadStatus::MessageTooLong error is issued
		//! @param size The new byte count
		void SetMaximumPayload(size_t size);

	  private:
		//! The maximum number of bytes (~= number of characters) a message can contain before a ReadStatus::MessageTooLong error is issued
		size_t MaxPayload = 4096;

		/*! @brief An internal function called during Initialisation. Checks if the socket is already occupied by another process. If so, either throws a message, or attempts a takeover
			@param forceAcquire If false and a socketfile already exists, the constructor will throw. If true, the object will attempt a takeover of the socket file, signalling the existing object to release the socket.
			@param gracePeriod The time given for the forceAcquire routine to work. If the socket is not released during this time period, the constructor will throw an error
			@throws std::runtime_error If the socketfile already exists, and forceAcquire is false
			@throws std::runtime_error If forceAcquire is true, but the gracePeriod elapses without the socketfile being released by the other process
		 */
		void QuerySocket(bool forceAcquire, milliseconds gracePeriod);

		/*! @brief The function called by QuerySocket to determine if a socket is already monitored
			@details Checks fs::exists on the SocketPath -- if it exists, it then checks if connect() works (i.e. sends a null-message). If this fails, then the file is `stale', and nobody is actually watching it -- so it can be taken over without penalty
			@param deleteStale If true, a stale-socket (a file which exists, but which no process is monitoring) gets deleted
			@returns true if an active process is monitoring the socket, else returns false
			@throws std::runtime_error if socket() throws an error (usually due to the OS running out of file descriptors)
		 */
		bool IsSocketClaimed(bool deleteStale = true);

		/*! @brief Opens the persistent file descriptor which will form the socket, and binds it to SocketPath.
			@throws std::runtime_error if socket(), bind() or listen() [the socket functions] returns an invalid state
		 */
		void BindSocket();

		/*! @brief The internal function called by Read() to signal that a message is incoming
			@details This is a hot-loop, and so will make a lot of pings on the OS
			@param deadline The clock time at which the process will return ReadStatus::Timeout
			@returns The file descriptor from which the message is coming, and/or the ReadStatus explaining why no descriptor was returned
		*/
		std::pair<socket_t, ReadStatus> AcceptIncoming(std::chrono::steady_clock::time_point deadline);

		/*! @brief If AcceptIncoming returned a valid filedescriptor, this opens the stream and reads bytes from it.
			@details For windows compatibility, this is a two-phase process: first a packet which is sizeof(uint32) is sent, which describes the size of the remaining packet. Then this number of bytes are read in.
			@param fd The socket file descriptor provided by AcceptIncoming
			@param deadline The clock time at which the process will return ReadStatus::Timeout
			@returns The MessageResult containing the message (if successful) and the ReadStatus
		*/
		MessageResult ReadStream(socket_t fd, std::chrono::steady_clock::time_point deadline);

		/*! @brief Reads the bytes from the socket-stream, and stores them in the dest* pointer
			@param fd The socket file descriptor provided by AcceptIncoming
			@param dest A pointer to the string where the message will be stored
			@param len The number of bytes to be read in
			@param deadline The clock time at which the process will return ReadStatus::Timeout
		 */
		ReadStatus ExtractFromStream(socket_t fd, char *dest, size_t len, std::chrono::steady_clock::time_point deadline);

		//! Rule of five deletion
		Listener(const Listener &) = delete;
		//! Rule of five deletion
		Listener &operator=(const Listener &) = delete;
		Listener(Listener &&other) noexcept;
		Listener &operator=(Listener &&other) noexcept;
	};
}; // namespace JSL::Async::Socket
