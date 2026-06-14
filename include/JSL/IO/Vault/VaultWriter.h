#pragma once
#include <JSL/IO/Filesystem.h>
#include <cstdio>
#include <iosfwd>
#include <optional>
#include <ostream>
#include <sstream>
#include <string_view>
#include <unordered_map>
#include <stdint.h>
namespace JSL::IO
{

	class VaultWriter
	{
		public:
			class Stream
			{
				protected:
					std::ostream * buffer;
				public:
					template<class T>
					std::ostream & operator<<(const T msg)
					{
						(*buffer) << msg;
						return (*buffer);
					}	
			};
		private:
			///	@brief An open stream into a single 'file' within the vault.
			class BufferedStream : public Stream
			{
				public:
					std::ostringstream StringBuffer;
					BufferedStream(){buffer = &StringBuffer;}
			};

			/// @brief An alternative stream for a vault file which is too large to buffer - instead it streams directly onto disk, and post-hoc updates the metadata with the final file size.
			/// @details Only a single DirectStream can exist per-vault
			class DirectStream : public Stream
			{
				public:
					std::ostream * FileStream;
					std::streampos Start;	
					void Open(std::ostream & diskStream)
					{
						buffer = &diskStream;
						Start = buffer->tellp();
					}
			};
		
			std::unordered_map<std::string, BufferedStream> Streams;
			Stream * MostRecentStream = nullptr;
			std::optional<DirectStream> LargeFile;
			
			bool Initialised = false;
			Policy Strictness = Policy::Strict;
			std::string Name;
			std::string TempName;
		public:
			//! Default initialiser, which places the object into an Uninitialised state. 
			VaultWriter();
				

			/*! @brief Initialises the object and immediately calls Open()
				@param archivePath The name of the archive on disk 
			 	@param policy The strictness policy applied to file operations. Policy::Strict throws errors, whilst Policy::Generous will overwrite files etc.
			*/
			VaultWriter(std::string_view archivePath, IO::Policy policy = Policy::Strict) ;

			~VaultWriter () noexcept;

			void Open(std::string_view path);
			
			void Close();
			Stream & operator[](const std::string & streamName);
			
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
			
			// Security settings	
			uint32_t UID = 1000; // uid of first normal human user. A good default as tar doesn't *really* need a uid (it's discarded when tar runs)
			uint32_t GID = 1000;
			std::string UName = "user";
			std::string GName = "user";
			uint32_t Permission = 0644; //standard linux -rw-r--r-- permission
	};
	// namespace Writer
	// {
	// 	struct Exporter
	// 	{
	// 		internal::TarHeader headerBlock;
	// 		std::string data;
	// 		size_t padding;
	// 	};

	// 	/*!	@brief An open stream into a single 'file' within the vault.
	// 	 */
	// 	class BufferedStream
	// 	{
	// 		private:
	// 			std::ostringstream Buffer;

	// 		public:
	// 			Exporter Export();
	// 			template<class T>
	// 			std::ostream & operator<<(const T msg)
	// 			{
	// 				Buffer << msg;
	// 				return Buffer;
	// 			}
	// 	};

	// 	/// @brief An alternative stream for a vault file which is too large to buffer - instead it streams directly onto disk, and post-hoc updates the metadata with the final file size.
	// 	/// @details Only a single DirectStream can exist per-vault
	// 	class DirectStream
	// 	{
	// 		private:
	// 			std::ostream * FileStream;
	// 			size_t StreamCount;

	// 		public:
	// 			DirectStream(std::ostream & diskStream);
	// 			Exporter Export();
	// 			template<class T>
	// 			std::ostream & operator<<(const T msg)
	// 			{
	// 				//get the current position of the filestream pointer
	// 				auto start = FileStream->tellp();
	// 				(*FileStream) << msg;
	// 				StreamCount += (FileStream->tellp() - start);
	// 			}
	// 	};

	// }
}
