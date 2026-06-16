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

	// Security settings	
	struct Security
	{
		uint32_t UID = 1000; // uid of first normal human user. A good default as tar doesn't *really* need a uid (it's discarded when tar runs)
		uint32_t GID = 1000;
		std::string UName = "user";
		std::string GName = "user";
		uint32_t Permission = 0644; //standard linux -rw-r--r-- permission
	};
	class VaultWriter
	{
		public:
			class Stream
			{
				protected:
					std::ostream * buffer;
				public:
					virtual ~Stream() = default;
					bool IsLarge = false;
					template<class T>
					std::ostream & operator<<(const T msg)
					{
						(*buffer) << msg;
						return (*buffer);
					}	
					virtual void Export(std::string_view name, std::ofstream & file, Security & security) = 0;
			};
		private:
			///	@brief An open stream into a single 'file' within the vault.
			class BufferedStream : public Stream
			{
				public:
					void Export(std::string_view name, std::ofstream & file, Security & security);
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
					DirectStream(std::ostream & diskStream)
					{
						IsLarge = true;
						buffer = &diskStream;
						Start = buffer->tellp();
					}
					void Export(std::string_view name, std::ofstream & file, Security & security);
			};
		
			std::map<std::string, std::unique_ptr<Stream>,std::less<>> Streams;
			Stream * MostRecentStream = nullptr;
			std::optional<std::string> LargeFile;
			
			bool Initialised = false;
			Policy Strictness = Policy::Strict;
			std::string Name;
			std::string TempName;
			std::ofstream OutputWriter;
		public:
			//! Default initialiser, which places the object into an Uninitialised state. 
			VaultWriter();
				

			/*! @brief Initialises the object and immediately calls Open()
				@param archivePath The name of the archive on disk 
			 	@param policy The strictness policy applied to file operations. Policy::Strict throws errors, whilst Policy::Generous will overwrite files etc.
			*/
			VaultWriter(std::string_view archivePath, IO::Policy policy = Policy::Strict) ;

			~VaultWriter () noexcept;

			void OpenVault(std::string_view path);
		
			Stream & NewFile(const std::string & name,bool openAsLarge = false);

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
			
			Security Settings;
	};
}
