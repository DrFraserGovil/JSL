#include "JSL/IO/Filesystem.h"
#include <JSL/IO/Vault/VaultReader.h>
#include <JSL/IO/Vault/VaultHeaders.h>
#include <JSL/internal/error.h>

#include <cstdio>
#include <fstream>
#include <string>
#include <cstring>
#include <string_view>
namespace JSL::IO
{
	/// STREAM FUNCTIONS
	void VaultReader::Stream::ForLineIn(std::function<void(std::string_view)> callback)
	{
		std::array<char, 10 * internal::BLOCK_SIZE> buffer;
		std::string cachedText = "";
		//move the stream pointer to the start of the file
		StreamCopy.seekg(FileStartPos);
		size_t remainingSize = Size;
		while (remainingSize > 0)
		{
			size_t last = 0;
			size_t pos = 0;
            size_t chunk_size = std::min(remainingSize, sizeof(buffer));
            StreamCopy.read(buffer.data(), chunk_size);
            remainingSize -= chunk_size;
			
			std::string_view newText(buffer.data(),buffer.data()+chunk_size);

			while ((pos=newText.find('\n',last)) != std::string_view::npos)
			{
				// Extract line, handling the merge if CachedText isn't empty
				if (!cachedText.empty())
				{
					cachedText.append(newText.substr(last, pos - last));
					callback(cachedText);
					cachedText.clear();
				}
				else
				{
					callback(newText.substr(last, pos - last));
				}
				last = pos + 1; // Move past '\n'
			}

			// Add remaining segment to cache
			if (last < newText.size())
			{
				cachedText.append(newText.substr(last));
			}
		}
	
		//any text left in the cache is a final non-newline terminated line, so pipe it in
		if (!cachedText.empty())
		{
			callback(cachedText);
		}
	}
	VaultReader::Stream::Stream(size_t size, std::streampos offset, std::ifstream &stream) : StreamCopy(stream)
	{
		FileStartPos = offset;
		Size = size;
	}
	std::string VaultReader::Stream::AsText()
	{
		std::string out;
		ForLineIn([&out](auto line){
			if (!out.empty())
			{
				out.insert(out.end(),'\n');
			}	
			out.append(line);
		});
		return out;
	}
	std::vector<std::string> VaultReader::Stream::AsLines()
	{
		std::vector<std::string> out;
		ForLineIn([&out](auto line){
			out.emplace_back(line);	
		});
		return out;
	}

	///READER FUNCTIONS
	VaultReader::VaultReader()
	{
		Initialised = false;
	}
	VaultReader::VaultReader(std::string_view filename, IO::Policy policy)
	{
		Open(filename,policy);
	}
	void VaultReader::Open(std::string_view filename, IO::Policy policy)
	{
		Name = filename;
		Strictness = policy;
		VaultStream.open(Name);
		if (!VaultStream.is_open())
		{
			JSL::internal::FatalError("Bad Vault Access", JSL_LOCATION) << "Could not open " << Name;
		}
		BuildIndex();
		Initialised = true;	
	}
	void VaultReader::Close()
	{
		if (Initialised)
		{
			VaultStream.close();
			FileIndex.clear();
			Initialised = false;
		}
	}
	 
	VaultReader::~VaultReader() noexcept { Close(); }

	///! @brief Reads in BLOCK_SIZE characters from the file, returning true if the expected number of bytes were red
	///! @param buffer The storage location for the read bytes
	///! @param file The file to read the data from
	///! @returns True if BLOCK_SIZE bytes were read (and thus False if the stream terminated unexpectedly, as tar files should be multiples of BLOCK_SIZE)
	bool readBlock(std::array<char, internal::BLOCK_SIZE> & buffer, std::ifstream & file)
	{
		file.read(buffer.data(),internal::BLOCK_SIZE);
		return file.gcount() == internal::BLOCK_SIZE;
	}

	void VaultReader::BuildIndex()
	{
		std::array<char,internal::BLOCK_SIZE> header;
		size_t zeroBlockCount = 0; 
		while (readBlock(header, VaultStream))
		{
			// Check for all-zero block (end of archive is (n>=2) such blocks)
			if (std::all_of(header.data(), header.data() + internal::BLOCK_SIZE, [](char c) { return c == '\0'; })) 
			{
				++zeroBlockCount;
				if (zeroBlockCount >= internal::ARCHIVE_END_BLOCK_COUNTS)
				{
					break;
				}
			}	
			else
			{
				zeroBlockCount = 0; //reset the count -- termination sequence is two contiguous null blocks
			

				// Parse metadata
				auto name  = std::string(header.data(), 100);
				name.erase(name.find('\0')); // Remove null padding
				char size_str[12];
				std::memcpy(size_str, header.data() + 124, 12); // File size starts at offset 124
				size_str[11] = '\0'; // Ensure null-termination
				size_t size = std::strtol(size_str,nullptr,8);

				std::streampos data_offset = VaultStream.tellg(); // Current position after reading the header

				FileIndex.try_emplace(name,Stream(size,data_offset,VaultStream));

				// Skip file data and padding
				size_t data_blocks = (size + internal::BLOCK_SIZE - 1) / internal::BLOCK_SIZE;
				VaultStream.seekg(data_blocks * internal::BLOCK_SIZE, std::ios::cur);
			}
		}

		if (zeroBlockCount != 2)
		{
			JSL::internal::FatalError("Corrupted Vault",JSL_LOCATION) << "Scan did not locate null termination sequence. The Vault " + Name + " is corrupted or incomplete";
		}
	}

	std::set<std::string> VaultReader::Files()
	{
		if (!Initialised)
		{
			JSL::internal::FatalError("Bad Vault",JSL_LOCATION) << "Cannot read from a vault before it is initialised";
		}
		std::set<std::string> names;
		for (auto pair : FileIndex)
		{
			names.insert(pair.first);
		}
		return names;
	}
	VaultReader::Stream & VaultReader::operator[](const std::string &file)
	{
		auto it = FileIndex.find(file);
		if (it == FileIndex.end())
		{
			JSL::internal::FatalError("Bad Vault Access",JSL_LOCATION) << "File `" + file + "` is not in vault " + Name;
		}
		return it->second;
	}

	void VaultReader::ForLineIn(const std::string &file, std::function<void(std::string_view)> callback)
	{
		//Delegate to the index function for existence checks etc.
		operator[](file).ForLineIn(callback);
	}
} // namespace JSL::IO
