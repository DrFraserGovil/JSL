#pragma once
#include "SocketBase.h"
#include <string_view>
namespace JSL::Async::Socket
{
	//! @brief Sends (stringified) messages to its paired socket
	class Broadcaster : public internal::SocketBase
	{
	  public:
		//! @brief A blank constructor, placing the object into an uninitialised state. Will throw errors when used unless SetTarget is called
		Broadcaster();

		/*! @brief Constructs a Broadcaster object and attempts to pair it with the socket matching the input name
			@param socketName The identifier (not a full path) to the paired socket. This should be the same name used to construct the Listener
		*/
		Broadcaster(std::string_view socketName);

		/*! @brief Attempts to create a connection to a socket with matching identifier.
			@details If the Broadcaster was already initialised, or had a previous target, this overrides it
			@throws std::runtime_error if the socketName is badly formed (empty, contains non-filesystem characters, too long)
		*/
		void SetTarget(std::string_view socketName);

		/*! @brief Places the message into the queue associated with the target socket
			@param message The message to send to the paired socket
			@throws std::runtime_error If the socket has not been paired, either via the constructor, or a SetTarget() call
			@throws std::runtime_error If the paired socket has not registered a socketfile in the expected location
		 */
		bool Transmit(std::string_view message);

	  private:
		//! Rule of five deletion
		Broadcaster(const Broadcaster &) = delete;
		//! Rule of five deletion
		Broadcaster &operator=(const Broadcaster &) = delete;
		//! Rule of five deletion
		Broadcaster(Broadcaster &&other) noexcept;
		//! Rule of five deletion
		Broadcaster &operator=(Broadcaster &&other) noexcept;
	};
}; // namespace JSL::Async::Socket
