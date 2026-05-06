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
namespace JSL::Async
{

	namespace internal
	{
		//this is a wrapper with a rule-of-5 that ensures move semantics work etc. 
		struct FileDescriptor
		{
			int FD =-1;
			std::filesystem::path Path;
			bool IsClient = true; 
			FileDescriptor() = default;
			FileDescriptor(int fd, std::filesystem::path path) : FD(fd), Path(path), IsClient(false){};
			~FileDescriptor();
			FileDescriptor& operator=(const FileDescriptor&) = delete;
			FileDescriptor(FileDescriptor && other) noexcept : FD(other.FD), Path(other.Path), IsClient(other.IsClient){other.FD = -1; other.Path = "";};
			FileDescriptor & operator=(FileDescriptor && other) noexcept { FD = other.FD; Path = other.Path; IsClient = other.IsClient; other.FD = -1; other.Path = "";return *this;};
			operator int() const { return FD; }
			operator std::filesystem::path() const {return Path;};
			operator std::string() const {return Path.string();};
		};
	}

	class Antenna
	{
		public:

			/*!	@brief Construct a persistent antenna which listens for incoming messages
				@param identifier A string which identifies the object; those wanting to send messages to this socket need to know this name in order to communicate. The socket is hosted at a location on disk (usually /tmp/[identifier]). If an Antenna with this name already exists, the system will check if there is an active process using it: if so, returns nullopt. If the socket is 'stale', it is taken over by this object. 
				@param forceAcquire If true, does not return nullopt on encountering an occupied socket. Instead, issues a 'shutdown' command to the existing socket, and then takes over.
				@param gracePeriod The time (in ms) allowed for a process to shutdown before the forceAcquire protocol takes place 
			*/
			static std::optional<Antenna> Create(std::string_view identifier, bool forceAcquire = false,std::chrono::milliseconds gracePeriod = std::chrono::milliseconds(50));


			/*!
				@brief Sends a message to an Antenna with matching identifier
				@param identifier The name of the Antenna which is being reached. If no matching antenna (or external process) is found, an error is thrown
				@param msg The message to be communicated
				@param timeout The time to wait for an acknowledgement from the reciever that the message was returned. 
				@returns True if the message was acknowledged, false otherwise
			*/
			static bool Transmit(std::string_view identifier, std::string_view msg, double timeout=2);


			//state queries
			bool IsActive();
			int GetDescriptor(){return Resources;}
			std::filesystem::path GetPath(){return Resources;}
			std::string GetID(){return Identifier;}
			
			//comms functions
			std::string Read();

			/*!
				@brief This is mostly just a wrapper for the Socket::Hotline  function without needing to keep the identifier around. 
			*/
			class Hotline
			{
				public:
					static std::optional<Hotline> Create(std::string_view identifier, double timeout);
					Hotline(const Antenna & host);

					bool Send(std::string_view msg);
					double Timeout;// intentionally public so the user can change it
					Hotline() = default;
				private:
				std::string ID; //private since the user shouldn't change this
				

				
			};
			void SetTimeout(double seconds);
			void Deactivate(){Active = false;}
		private:
			Antenna();
			double Timeout = 2;
			void Connect(std::string_view path);

			std::string Identifier;
			sockaddr_un Address;
			internal::FileDescriptor Resources;

			//Indicates that the socket file existed at initialisation time (i.e. an active process is monitoring messages to the socket)
			bool AlreadyMonitored=false;

			//Indicates that the socket is in a valid state and has not recieved a shutdown order. 
			bool Active = false;
			
			static Antenna TransmitClient(std::string_view identifier);
			static Antenna ReadClient(int filedescriptior);

			std::string GetMessage();
			void Bind();
			bool Send(std::string_view msg);
			static constexpr size_t Buffer = 4096;		
	};


}