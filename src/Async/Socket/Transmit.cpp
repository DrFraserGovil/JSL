#include <JSL/Async/Socket/Broadcaster.h>
#include <JSL/Async/Socket/Transmit.h>

namespace JSL::Async::Socket
{

	bool Transmit(std::string_view targetSocket, std::string_view msg)
	{
		Broadcaster B(targetSocket);

		return B.Transmit(msg);
	}

} // namespace JSL::Async::Socket
