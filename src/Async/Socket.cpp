#include <JSL/Async.h>
#include <JSL/Strings/Cases.h>
#include <JSL/internal/error.h>
#include <thread>
#include <chrono>
namespace JSL
{
	//this should only be 
	Socket::Socket()
	{
		FileDescriptor = -1;
		SocketMonitored = false;
		Bindable = false; // this method is only used by temporaries, who cannot be bound 
		SetTimeout(2);
	}

	void Socket::Initialise(const std::string & path, double timeout)
	{
		Path = path;
		Bindable = true;
		Address = {};
		Address.sun_family = AF_UNIX;
		if (path.length() < sizeof(Address.sun_path)-1)
		{
			path.copy(Address.sun_path,path.length()); //address was zeroed out so no need to null terminate
		}
		else
		{
			throw std::runtime_error(path + "too long for a Unix Domain Socket");
		}

		FileDescriptor = socket(AF_UNIX,SOCK_SEQPACKET,0); //use STREAM not SEQPACKET for windows compatibility 
		OwnsFile = false;
		bool pathExists = std::filesystem::exists(path);
		SocketMonitored = (connect(FileDescriptor, (struct sockaddr*)&Address, sizeof(Address)) == 0);
		
		if (pathExists && !SocketMonitored)
		{
			LOG(DEBUG) << "Detected a stale socket file: inheritance complete";
		}
		SetTimeout(timeout);
	}

	Socket::Socket(const std::string & path, double timeout)
	{
		Initialise(path,timeout);
	}

	std::optional<Socket> Socket::Broadcaster(const std::string & path, double timeout)
	{
		Socket out(path,timeout);
		if (out.SocketMonitored)
		{
			return std::move(out);
		}
		return std::nullopt;
	}
	std::optional<Socket> Socket::Antenna(const std::string & path,  double timeout, bool forceAcquire, size_t gracePeriod)
	{
		Socket out(path,timeout);
		if (out.SocketMonitored && !forceAcquire)
		{
			return std::nullopt;

		}

		if (out.SocketMonitored)
		{
			//send a message that we're taking over
			LOG(DEBUG) << "Attempting hostile takeover";
			out.Send("exit");
			using namespace std::literals::chrono_literals;
			std::this_thread::sleep_for(static_cast<std::chrono::milliseconds>(gracePeriod));
			out.Initialise(path,timeout); //try again!
		}
		out.UniqueBind();
		out.Listen();
		return std::move(out);
	}

	Socket::Socket(Socket&& other) noexcept :  Address(other.Address),FileDescriptor(other.FileDescriptor),Path(std::move(other.Path)),SocketMonitored(other.SocketMonitored), OwnsFile(other.OwnsFile), Active(other.Active), Bindable(other.Bindable), Timeout(other.Timeout) 
    {
        other.FileDescriptor = -1; // Mark as "moved out"
		other.Bindable = false;
    }

	// Move Assignment Operator
    Socket& Socket::operator=(Socket&& other) noexcept
    {
        if (this != &other) 
        {
            Close(); // Close our current FD before taking the new one
            
            FileDescriptor = other.FileDescriptor;
            SocketMonitored = other.SocketMonitored;
            Address = other.Address;
            Path = std::move(other.Path);
			Timeout = other.Timeout;
			
            other.FileDescriptor = -1; // Mark as "moved out"
			other.Bindable = false;
        }
        return *this;
    }

	Socket::~Socket()
	{
		if (FileDescriptor != -1)
		{
			Close();
		}
		if (OwnsFile)
		{
			Unbind();
		}
	}

	bool Socket::IsActive()
	{
		return Active;
	}

	void Socket::Bind()
	{
		if (!Bindable)
		{
			internal::FatalError("Attempt to bind non-bindable socket",JSL_LOCATION) << "This socket (" << FileDescriptor << ", " << Path.string() << ") is a  non-bindable temporary";
		}
		int result = bind(FileDescriptor,reinterpret_cast<const sockaddr*>(&Address), sizeof(Address));
		if (result == -1)
		{
			internal::FatalError("Bad socket bind",JSL_LOCATION) << "Could not bind socket " << Path.string();
		}
		SocketMonitored = true;
		OwnsFile = true;
		Active = true;
		LOG(DEBUG) << "Socket bind complete: " << SocketMonitored << OwnsFile << Active;
	}

	void Socket::UniqueBind()
	{
		Unbind();
		Bind();
	}

	void Socket::Unbind()
	{
		unlink(Path.string().c_str());
		SocketMonitored = false;
	}

	void Socket::Close()
	{
		if (FileDescriptor != -1)
		{
			close(FileDescriptor);
			FileDescriptor = -1;
			Bindable = false;
		}
	}

	void Socket::Listen()
	{
		listen(FileDescriptor,5); // 5 is standard for interfaces not expecting heavy loads
	}

	void Socket::SetTimeout(double seconds)
	{
		Timeout.tv_sec = (int)seconds;  // Seconds
		Timeout.tv_usec = (seconds - (int)seconds)*1000000;; // Microseconds

		setsockopt(FileDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&Timeout, sizeof(Timeout));
	}
	void Socket::SetTimeout(timeval time)
	{
 		Timeout = time;

		setsockopt(FileDescriptor, SOL_SOCKET, SO_RCVTIMEO, (const char*)&Timeout, sizeof(Timeout));
	}

	void Socket::Send(const std::string & msg)
	{
		if (FileDescriptor == -1) internal::FatalError("Bad socket",JSL_LOCATION) << "Cannot write to an expired socket";
		//two stage send for SOCK_STREAM
		
		if (msg.length() > UINT32_MAX)
		{
			internal::FatalError("Message length exceeded",JSL_LOCATION) << "Message exceeds buffer size";
		}
		
		uint32_t length = static_cast<uint32_t>(msg.length());
    


		//send the actual message
         if (write(FileDescriptor, msg.c_str(),length) == -1)
        {
            internal::FatalError("Failed to send message",JSL_LOCATION) << "Command '" << msg << "' not sent";
        }
	}
	std::string Socket::SendAndReply(const std::string & msg, double timeout)
	{
		SetTimeout(timeout);
		Send(msg);

		return GetMessage();
	}

	std::string Socket::Read()
	{
		auto [msg,client] = ReadAndReply(); 
		client.Close();
		return msg;
	}

	std::pair<std::string,Socket> Socket::ReadAndReply()
	{
		LOG(DEBUG) << "Attempting to read from socket";
		if (FileDescriptor == -1) internal::FatalError("Bad socket",JSL_LOCATION) << "Cannot read from an expired socket";
		int fd = accept(FileDescriptor,nullptr,nullptr);

		auto client = Client(fd,Path);
		client.SetTimeout(Timeout);
		client.Active = true;
		std::string msg = client.GetMessage();
		Active = client.Active;
		return {msg,std::move(client)};
	}

	void Socket::Sync(Socket & pair)
	{
		pair.SetTimeout(Timeout);	
	}
	Socket Socket::Client(int filedescriptior, std::filesystem::path path)
	{
		Socket out;
		out.FileDescriptor = filedescriptior;
		out.Path = path;
		return out;
	}

	int Socket::Descriptor()
	{
		return FileDescriptor;
	}

	std::string Socket::GetMessage()
	{
	

		std::vector<char> buffer(Buffer, 0);
		ssize_t nbytes = read(FileDescriptor, buffer.data(), buffer.size());
		
		if (nbytes == 0)
		{
			return "";
		}
		if (nbytes < 0)
		{
			if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR) {
                return ""; 
            }
			internal::FatalError("Socket read error", JSL_LOCATION) 
				<< "Failed to read from socket " << FileDescriptor 
				<< " at " << Path.string();
		}


		
		std::string msg(buffer.data(),nbytes);
		if (JSL::iEquals(msg,"exit") || JSL::iEquals(msg,"shutdown"))
		{
			Active = false;
		}
		return msg;
	}
}