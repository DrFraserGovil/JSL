
#pragma once
#include <string_view>

namespace JSL::Async::Socket
{

	/*!
		@brief Sends a message to a Socket with matching identifier
		@param identifier The name of the Socket which is being reached.
		@throws std::runtime_error If no matching Socket is found, an error is thrown
		@param msg The message to be communicated
		@param timeout The time to wait (in seconds) for an acknowledgement from the reciever that the message was returned.
		@returns True if the message was acknowledged, false otherwise
	*/
	bool Transmit(std::string_view targetSocket, std::string_view msg, double timeout = 2);

} // namespace JSL::Async::Socket
