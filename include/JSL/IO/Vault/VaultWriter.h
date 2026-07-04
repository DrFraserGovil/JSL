#pragma once
#include <JSL/IO/Filesystem.h>
#include <cstdio>
#include <functional>
#include <iosfwd>
#include <memory>
#include <optional>
#include <ostream>
#include <sstream>
#include <string_view>
#include <fstream>
#include <map>
#include <stdint.h>
namespace JSL::IO
{

	/// @brief A POD for holding the standard POSIX file security data. The default values are 'sensible defaults'	
	struct Security
	{
		uint32_t UID = 1000; //!< uid of first normal human user. A good default as tar doesn't *really* need a uid (it's discarded when tar runs)
		uint32_t GID = 1000;
		std::string UName = "user"; 
		std::string GName = "user";
		uint32_t Permission = 0644; //!< standard linux "-rw-r--r--" permission
	};
	
	/// @brief Enables the user to stream data directly into a TAR-like archive on disk.
	/// @details Most data is stored in-memory (via BufferedStream objects) until the Writer is closed; although we support **one** DirectStream per vault which prevents having to keep a large file buffered on disk, whilst also allowing us to keep the process of writing the metadata (which goes *before* the file, and thus can't be written until the stream is finalised) simple. 
	class VaultWriter
	{
		public:
			/// @brief A pure virtual interface for the various types of stream, which allow streaming in of arbitrary serialisable types
			class Stream
			{
				protected:
					/// A pointer to the buffer, which may be an ostringstream or an ofstream, depending on the child (so we're doing some slightly naughty pointer things here)
					std::ostream * buffer;
				public:
					virtual ~Stream() = default;
					
					/// This is false unless the child type is a DirectStream, in which case it is true. This is just to allow rapid scanning without needing to do string comparisons.
					bool IsLarge = false;
					
					/// The streaming function; templated to allow any input type that the underlying stream can support
					/// @param msg Data to be streamed into the vault
					/// @returns A reference to the stream, allowing chaining of input  
					template<class T>
					std::ostream & operator<<(const T msg)
					{
						(*buffer) << msg;
						return (*buffer);
					}	
					
					/// @brief Ensure that the data ends up on the disk with the correct tar-formatted header
					/// @param name The name of the `file' associated with the stream
					/// @param file The filestream to write output to
					/// @param security The posix file permissions to assign to the file
					virtual void Export(std::string_view name, std::ofstream & file, Security & security) = 0;
			};
		private:
			///	@brief An open stream into a single 'file' within the vault.
			class BufferedStream : public Stream
			{
				public:
					/// @brief Write the header to file, and then follow it with the contents of the buffer
					/// @param name The name of the `file' associated with the stream
					/// @param file The filestream to write output to
					/// @param security The posix file permissions to assign to the file
					void Export(std::string_view name, std::ofstream & file, Security & security);
					
					/// @brief The buffer where the contents of the 'file' sit until the vault closes
					std::ostringstream StringBuffer;
					
					/// Constructor which ties the StringBuffer to the buffer pointer. 
					BufferedStream(){buffer = &StringBuffer;}
			};

			/// @brief An alternative stream for a vault file which is too large to buffer - instead it streams directly onto disk, and post-hoc updates the metadata with the final file size.
			/// @details Only a single DirectStream can exist per-vault
			class DirectStream : public Stream
			{
				public:
					/// A pointer to the FileStream which is used to write the data to file
					std::ostream * FileStream;
					/// The position of the FileStream **after the header has been written** (i.e. the position of the start of the actual data)
					std::streampos Start;
					
					DirectStream(std::ostream & diskStream)
					{
						IsLarge = true;
						buffer = &diskStream;
						Start = buffer->tellp();
					}
					/// @brief Overwrites the original 'empty header' now that the true size of the data is known
					/// @details Requires moving the filestream pointer around to overwrite the header data without touching the 'buffer'
					/// @param name The name of the `file' associated with the stream
					/// @param file The filestream to write output to
					/// @param security The posix file permissions to assign to the file
					void Export(std::string_view name, std::ofstream & file, Security & security);
			};
	
			/// A map of the open 'files' within the vault
			std::map<std::string, std::unique_ptr<Stream>,std::less<>> Streams;
			
			/// A pointer to the most recent file used by the vault, used when data is piped to the vault without specifying a file
			Stream * MostRecentStream = nullptr;
			
			/// If this exists, it is the name of the file in the Streams map which is set as a DirectStream, rather than a BufferedStream
			std::optional<std::string> LargeFile;
		
			/// Set to true when OpenVault() has completed, indicates that the vault can be written to without error
			bool Initialised = false;
			
			/// A file policty which indicates if errors should be thrown (strict) or silently swallowed (Generous)
			Policy Strictness = Policy::Strict;
			
			/// The filename of the vault
			std::string Name;
			
			/// The name that the vault writes to whilst under construction (usually Name + ".part"), to prevent corruption overwrites
			std::string TempName;
			
			/// The filestream used to write to disk
			std::ofstream OutputWriter;
		public:
			//! Default initialiser, which places the object into an Uninitialised state. 
			VaultWriter();
				

			/*! @brief Initialises the object and immediately calls Open()
				@param archivePath The name of the archive on disk 
			 	@param policy The strictness policy applied to file operations. Policy::Strict throws errors, whilst Policy::Generous will overwrite files etc.
			*/
			VaultWriter(std::string_view archivePath, IO::Policy policy = Policy::Strict) ;

			/// If Initialised is true, this calls Close() to ensure that the vault is written to disk when it passes out of scope.
			~VaultWriter () noexcept;

			/// Initialises the vault at the specified path
			/// @param path The requested filename of the Vault on disk
			void OpenVault(std::string_view path);
	
			/// Opens a new 'file' in the archive, and associates a stream with it
			/// @param name The filename
			/// @param openAsLarge If true, the file is set to stream directly to disk, rather than a buffer. Use this when the file is too large to reasonable keep entirely in memory until the vault closes. **Note:** Only a single file per vault may be set as large.
			/// @throws runtime_error If the file is already open within the file, and Strictness is set to Policy::Strict. The error is thrown under any policy if the openAsLarge value differs from that used to open the original file
			/// @returns The Stream object associated with the new file
			Stream & NewFile(const std::string & name,bool openAsLarge = false);

			/// Closes the archive, and triggers the write-to-disk operation. Sets Initialised to False when complete.
			void Close();
			
			/// Access the stream associated with the requested file 
			/// @param fileName The file stream to access
			/// @throws runtime_error If fileName was not already created and the policy is Strict. If it is Generous, NewFile is implicitly called.  
			/// @return The Stream object associated with the requested file
			Stream & operator[](const std::string & fileName);
		
			/// Pipe contents into the most recently accessed stream
			/// @details 'Access' means either creating a file via NewFile, or requesting it via []
			/// @tparam T An arbitrary, serialisable type
			/// @param msg The data to stream to the file
			/// @returns A reference to the most recently accessed file stream, to allow chained streaming
			template<class T>
			Stream & operator<<(T msg)
			{
				if (!MostRecentStream)
				{
					throw std::logic_error("Attemptng to write to an archive before write stream open");
				}
				(*MostRecentStream) << msg;
				return (*MostRecentStream);
			}
	
			/// The POSIX security settings assigned to all files in the vault
			Security Settings;
			 
			/// Change the Strictness policy
			/// @param policy The value to assign to Strictness. 
			void SetPolicy(Policy policy);
	};
}
