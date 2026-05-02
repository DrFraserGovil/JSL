#include <JSL/Async.h>
#include <JSL/Strings/Cases.h>
#include <JSL/internal/error.h>
#include <thread>
#include <chrono>
namespace JSL
{
	namespace fs = std::filesystem;

	internal::FileDescriptor::~FileDescriptor()
	{
		if (FD != -1)
		{
			close(FD);
		}
		if (!IsClient)
		{
			unlink(Path.string().c_str());
		}	
	}

	//this should only be used by the internal methods
	Antenna::Antenna()
	{
		Resources = internal::FileDescriptor();
		AlreadyMonitored = false;
	}

	void Antenna::Connect(std::string_view identifier)
	{
		if (Resources.FD != -1)
		{
			close(Resources.FD);
			Resources.FD = -1; //in case socket fails 
		}
		Resources.Path = (fs::temp_directory_path() / identifier);
		auto path  = Resources.Path.string();
		Identifier = identifier;
		Address = {};
		Address.sun_family = AF_UNIX;
		if (path.length() < sizeof(Address.sun_path)-1)
		{
			path.copy(Address.sun_path,path.length()); //address was zeroed out so no need to null terminate
		}
		else
		{
			throw std::runtime_error(static_cast<std::string>(path) + "too long for a Unix Domain Socket");
		}

		int fd = socket(AF_UNIX,SOCK_SEQPACKET,0); //use STREAM not SEQPACKET for windows compatibility 

		bool pathExists = std::filesystem::exists(Resources.Path);

		if (pathExists)
		{
			AlreadyMonitored = (connect(fd, (struct sockaddr*)&Address, sizeof(Address)) == 0);
			
			if (AlreadyMonitored)
			{
				LOG(DEBUG) << "Connection established to existing socket";
			}
			else
			{
				LOG(DEBUG) << "Detected and took-over a stale socket file";
				Resources.IsClient = false;
			}
		}
		else
		{
			AlreadyMonitored = false;
			LOG(DEBUG) << "No socket file detected";
			Resources.IsClient = false;
		}	
		Resources.FD = fd;
	}

	std::optional<Antenna> Antenna::Create(std::string_view identifier, bool forceAcquire, std::chrono::milliseconds gracePeriod)
	{
		
		Antenna out;
		out.Connect(identifier);
		if (out.AlreadyMonitored && !forceAcquire)
		{
			return std::nullopt;

		}

		if (out.AlreadyMonitored)
		{
			//send a message that we're taking over
			LOG(DEBUG) << "Attempting hostile takeover";
			Transmit(identifier,"exit");
			using namespace std::literals::chrono_literals;
			std::this_thread::sleep_for(gracePeriod);
			out.Connect(identifier); //try again!
		}
		out.Bind();
		out.Active = true;
		return std::move(out);
	}

	bool Antenna::IsActive()
	{
		return Active;
	}

	void Antenna::Bind()
	{

		if (fs::exists(Resources.Path))
		{
			unlink(Resources.Path.string().c_str());
		}

		
		if (bind(Resources.FD,reinterpret_cast<const sockaddr*>(&Address), sizeof(Address)) == -1)
		{
			internal::FatalError("Bad socket bind",JSL_LOCATION) << "Could not bind socket " << Resources.Path.string();
		}
		if (listen(Resources.FD,5) == -1) // 5 is standard for interfaces not expecting heavy loads
		{
			internal::FatalError("Bad socket listen", JSL_LOCATION) << "Could not listen to socket (although could bind)" << Resources.Path.string();
		}
		
		LOG(DEBUG) << "Antenna-Socket bind complete";
	}

	


	void Antenna::SetTimeout(double seconds)
	{
		Timeout = seconds;
		timeval t;
		t.tv_sec = (int)seconds;  // Seconds
		t.tv_usec = (seconds - (int)seconds)*1000000; // Microseconds

		setsockopt(Resources.FD, SOL_SOCKET, SO_RCVTIMEO, (const char*)&t, sizeof(t));
	}


	bool Antenna::Transmit(std::string_view identifier, std::string_view msg, double timeoutSeconds)
	{
		auto client = Antenna::TransmitClient(identifier);

		if (!client.AlreadyMonitored)
		{
			return false;
		}

		client.SetTimeout(timeoutSeconds);

		bool success = client.Send(msg);
		if (!success) {return false;}

		//await response
		std::string response = client.GetMessage();

		return !response.empty();

	}



	bool Antenna::Send(std::string_view msg)
	{
		if (Resources.FD == -1) internal::FatalError("Bad socket",JSL_LOCATION) << "Cannot write to an expired socket";
		//two stage send for SOCK_STREAM
		
		if (msg.length() > UINT32_MAX)
		{
			internal::FatalError("Message length exceeded",JSL_LOCATION) << "Message exceeds buffer size";
		}
		
    
		//send the actual message
		if (write(Resources.FD, msg.data(),msg.length()) == -1)
        {
           LOG(WARN) << "Failed to send message " << msg;
		   return false;
        }
		return true;
	}


	std::string Antenna::Read()
	{
		LOG(DEBUG) << "Attempting to read from socket";
		if (Resources.FD == -1) internal::FatalError("Bad socket",JSL_LOCATION) << "Cannot read from an expired socket";
		
		int fd = accept(Resources.FD,nullptr,nullptr);
		auto client = ReadClient(fd);
		client.SetTimeout(Timeout);
		client.Active = true;
		std::string msg = client.GetMessage();
		Active = client.Active;
		if (!msg.empty())
		{ 	//empty messages are usually connect attempts that require no response
			client.Send("Acknowledge " + msg);
		}
		else
		{
			LOG(WARN) << "Empty message recieved. \nLikely an attempt to claim the socket";
		}

		return msg;
	}


	Antenna Antenna::TransmitClient(std::string_view identifier)
	{
		Antenna out;
		out.Connect(identifier);
		out.Resources.IsClient = true;
		return out;
	}

	Antenna Antenna::ReadClient(int filedescriptior)
	{
		Antenna out;
		out.Resources.FD = filedescriptior;
		out.Resources.IsClient = true;
		LOG(DEBUG) << "Read client generated";
		return out;
	}



	std::string Antenna::GetMessage()
	{
	

		std::vector<char> buffer(Buffer, 0);
		ssize_t nbytes = read(Resources.FD, buffer.data(), buffer.size());
		
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
				<< "Failed to read from socket " << Resources.FD 
				<< " at " << Resources.Path.string();
		}


		
		std::string msg(buffer.data(),nbytes);
		if (JSL::iEquals(msg,"exit") || JSL::iEquals(msg,"shutdown"))
		{
			Active = false;
		}
		return msg;
	}


	std::optional<Antenna::Hotline> Antenna::Hotline::Create(std::string_view identifier, double timeout)
	{
		Antenna::Hotline out;
		out.ID = identifier;
		out.Timeout = timeout;

		fs::path p = fs::temp_directory_path() / identifier;

		//we don't call `connect' here, because we don't want to trigger an unwanted 'ping' on the poll. 
		// instead we implicitly lazily connect in the TransmitClient 
		if (!fs::exists(p))
		{
			return std::nullopt;
		}
		
		return out;
	}

	Antenna::Hotline::Hotline(const Antenna & host)
	{
		ID = host.Identifier;
		Timeout = host.Timeout;
		//no further checks -- the face we have a host means they'd guarantee to pass
	}

	bool Antenna::Hotline::Send(std::string_view msg)
	{
		LOG(DEBUG) << "Hotline transmitting " << msg << " to " << ID;
		return Antenna::Transmit(ID,msg,Timeout);
	}
}