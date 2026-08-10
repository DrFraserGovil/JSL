
#pragma once
#include <string_view>

namespace JSL::Async::Socket
{

	/*!
		@brief Sends a message to a Socket with matching identifier
		@param identifier The name of the Socket which is being reached.
		@throws std::runtime_error If no matching Socket is found, an error is thrown
		@param msg The message to be communicated
		@returns True if the message was acknowledged, false otherwise
	*/
	bool Transmit(std::string_view targetSocket, std::string_view msg);

} // namespace JSL::Async::Socket
