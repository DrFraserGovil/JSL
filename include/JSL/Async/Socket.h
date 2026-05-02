#pragma once

#if defined(_WIN32) || defined(_WIN64)
	///for windows?
#else
	#include <sys/socket.h>
	#include <sys/un.h>
#endif

#include <filesystem>
#include <string>
#include <optional>
namespace JSL
{
	class Socket
	{
		public:
		
			//constructors & factories
			Socket(std::string_view path, double timeout=2);
			static std::optional<Socket> Broadcaster(std::string_view path, double timeout=2);
			static std::optional<Socket> Antenna(std::string_view path, double timeout=2,bool forceAcquire = false,size_t gracePeriod = 50);
			static Socket Null();
			
			//modify behaviour
			void Initialise(std::string_view path, double timeout=2);
			void SetTimeout(double seconds);
			void SetTimeout(timeval time);
			
			//state queries
			bool IsActive();
			int Descriptor();
			
			//comms functions
			void Send(std::string_view msg);
			std::string SendAndReply(std::string_view msg, double timeout=2);
			std::string Read();
			std::pair<std::string,Socket> ReadAndReply();
			
			//rule of 5
			~Socket();
			Socket(const Socket &) = delete;
			Socket & operator=(const Socket &) = delete;
			Socket(Socket&& other) noexcept;
			Socket& operator=(Socket&& other) noexcept;		
			std::filesystem::path GetPath(){return Path;};
			
			void Sync(Socket & pair);
		private:
			Socket();

			sockaddr_un Address;
			int FileDescriptor;
			std::filesystem::path Path;
			

			//Indicates that the socket file existed at initialisation time (i.e. an active process is monitoring messages to the socket)
			bool SocketMonitored=false;

			//Indicates that we are responsible for the socket file, and are responsible for closing it 
			bool OwnsFile = false;

			//Indicates that the socket is in a valid state and has not recieved a shutdown order. 
			bool Active = false;

			//Indicates that the socket has been placed into a state where it can be bound to a socket file (used as a proxy for temporary sockets)
			bool Bindable=false;

			timeval Timeout;
			
			static Socket Client(int filedescriptior, std::filesystem::path path);
			std::string GetMessage();
			void UniqueBind();
			void Listen();
			void Bind();
			void Close();
			void Unbind();
			const size_t Buffer = 4096;
	};
}