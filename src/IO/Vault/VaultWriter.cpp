#include "JSL/IO/Filesystem.h"
#include <JSL/IO/Vault/VaultWriter.h>
#include <JSL/IO/Vault/VaultHeaders.h>
#include <JSL/internal/error.h>
#include <algorithm>
#include <filesystem>
#include <charconv>
#include <fstream>
#include <memory>
#include <string_view>
#include <span>
namespace JSL::IO
{
	VaultWriter::VaultWriter()
	{
		Initialised = false;
	}
	
	VaultWriter::VaultWriter(std::string_view path, Policy policy)
	{
		Strictness = policy;
		OpenVault(path);	
	}

	void VaultWriter::OpenVault(std::string_view path)
	{
		bool strictmode = (Strictness == Policy::Strict);

		//If the vault is already open, close it then re-open. If in strictmode, this causes an error 
		if (Initialised)
		{
			if (strictmode)
			{
				JSL::internal::FatalError("Vault-Strictmode Error",JSL_LOCATION) <<"Cannot open a stream which is already open";
			}
			Close();	
		}

		Name = path;	
		TempName = Name + ".part"; //we write here then mv later
		
		if (strictmode && std::filesystem::exists(Name))
		{
				JSL::internal::FatalError("Vault-Strictmode Error",JSL_LOCATION) << Name << " already exists; cannot open new vault here";
		}
		
		OutputWriter.open(TempName,std::ios::out|std::ios::binary);
		if (!OutputWriter.is_open())
		{
			JSL::internal::FatalError("Vault Error",JSL_LOCATION) << "Could not open " << TempName;
		}
		Initialised = true;

			
	}

	//templated as these are all arrays of different sizes
	template<typename T>
	void to_octal(T & arr, uint64_t value)
	{
		std::fill(arr.begin(), arr.end(), '0');
		
		auto [ptr, ec] = std::to_chars(arr.data(), arr.data() + arr.size() - 1, value, 8);
		
		// We need to shift it to the right to keep the leading zeros we just filled.
		if (ec == std::errc{})
		{
			size_t written = ptr - arr.data();
			size_t capacity = arr.size() - 1;
			if (written < capacity)
			{
				std::move_backward(arr.data(), arr.data() + written, arr.data() + capacity);
				std::fill_n(arr.data(), capacity - written, '0');
			}
		}
		arr.back() = '\0'; // Ensure termination
	}
	
	void writeHeader(std::string_view name, std::ofstream & file, Security & security, size_t writeSize)
	{
		internal::TarHeader header;
	
		//save the name, truncating if it is above the max size
		auto name_size = std::min(name.size(), header.name.size() - 1);
		std::copy_n(name.begin(), name_size, header.name.data());
	
		//prepare the header array with some complex octal conversions
		to_octal(header.size, writeSize);
		to_octal(header.mode, security.Permission);
		to_octal(header.uid, security.UID);
		to_octal(header.gid, security.GID);
		to_octal(header.mtime, std::time(nullptr));
		std::copy_n(security.UName.c_str(), std::min(security.UName.size(), size_t(31)), header.uname.data());
		std::copy_n(security.GName.c_str(), std::min(security.GName.size(), size_t(31)), header.gname.data());

		// Generate the checksum
		// We must treat the checksum field as spaces during the sum calculation
		std::fill(header.chksum.begin(), header.chksum.end(), ' ');
		unsigned int sum = 0;
		auto bytes = std::as_bytes(std::span{&header, 1});
		for (auto b : bytes)
		{
			sum += static_cast<unsigned int>(b);
		}
		std::to_chars(header.chksum.data(), header.chksum.data() + 6, sum, 8);
		header.chksum[6] = '\0';
		header.chksum[7] = ' ';


		
		file.write(reinterpret_cast<const char*>(&header), sizeof(header));
	}

	void VaultWriter::BufferedStream::Export(std::string_view name, std::ofstream & file, Security & security)
	{
		//compute the size of the data in the buffer
		std::string data = std::move(StringBuffer).str();
		size_t writeSize = data.size();

		writeHeader(name, file, security, writeSize);
		// ///NOW DO THE WRITING
		file.write(data.data(),writeSize);
		
		size_t padding = (512u - (writeSize % 512u)) & 511u;
		if (padding > 0)
		{
			file.write(internal::zeroBuffer,padding);
		}
	}
	
	void VaultWriter::DirectStream::Export(std::string_view name, std::ofstream & file, Security & security)
	{
		//compute how much we wrote to disc
		size_t sizeWritten = file.tellp() - Start;
		file.seekp(Start); //send the pointer back to the beginning	
	
		//then overwrite the header with the new one, now we know what the actual size written was
		writeHeader(name, file, security, sizeWritten);
		
		//then jump back to the end, and add the padding as normal
		file.seekp(0,std::ios::end);
		size_t padding = (512u - (sizeWritten % 512u)) & 511u;
		if (padding > 0)
		{
			file.write(internal::zeroBuffer,padding);
		}
	}

	void VaultWriter::Close()
	{
		if (!Initialised)
		{
			return; //do nothing if not initialised
		}
		
		//LARGE FILE FIRST
		if (LargeFile)
		{
			Streams[LargeFile.value()]->Export(LargeFile.value(), OutputWriter, Settings);
		}
		
		for (auto & [name,stream] : Streams)
		{
			if (!stream->IsLarge)
			{
				stream->Export(name, OutputWriter, Settings);
			}
		}
		
		OutputWriter.write(internal::END_OF_ARCHIVE,internal::ARCHIVE_END_BLOCK_COUNTS * internal::BLOCK_SIZE);
		OutputWriter.close();
		Streams.clear();
		Initialised = false;
		MostRecentStream = nullptr;
		
		std::filesystem::rename(TempName,Name);	
	}
	
	VaultWriter::Stream & VaultWriter::NewFile(const std::string & name,bool openAsLarge)
	{
		if (!Initialised)
		{
			JSL::internal::FatalError("Bad archive access",JSL_LOCATION) << "Cannot create a file before initialising the archive";
		}
		if (Streams.contains(name))	
		{
			if (Strictness == Policy::Strict)
			{
				JSL::internal::FatalError("Strictmode Error",JSL_LOCATION) << "Cannot create " << name << " as this already exists in the archive";	
			}
			if (openAsLarge != Streams[name]->IsLarge)
			{
				JSL::internal::FatalError("Large File Error",JSL_LOCATION) << "Cannot change large-file status of an already open file (" << name << ")";
			}
		}
		else //name not in dict - so need to create it
		{
			if (openAsLarge)
			{
				if (LargeFile)
				{
					JSL::internal::FatalError("Large File Error",JSL_LOCATION) << "Cannot open " << name << " as a large file, as " << LargeFile.value() << " already exists, and only one file per vault can be marked as large";
				}
				writeHeader(name, OutputWriter, Settings, 0); //initialises the header so that the OutputWriter points to the correct location
				Streams[name] = std::make_unique<DirectStream>(OutputWriter);
			}	
			else 
			{
				Streams[name] = std::make_unique<BufferedStream>();
			}	
		}		
		MostRecentStream = Streams[name].get();
		return (*Streams[name]);
	}

	VaultWriter::Stream & VaultWriter::operator[](const std::string & streamName)
	{
		if (!Initialised)
		{
			JSL::internal::FatalError("Bad archive access",JSL_LOCATION) << "Cannot access a stream before initialising the archive";
		}
	
		if (!Streams.contains(streamName))
		{
			return NewFile(streamName);
		}
		
		MostRecentStream = Streams[streamName].get();
		return (*MostRecentStream);
	}
}
