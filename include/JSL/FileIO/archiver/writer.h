#pragma once
#include <JSL/FileIO/archiver/base.h>
#include <JSL/FileIO/archiver/writeStream.h>
#include <JSL/internal/error.h>
#include <unordered_map>
namespace JSL::Archiver
{
	//templte specialisation for the vault-writer
	template<>
	class Vault<Mode::Write> : public internal::VaultBase
	{
		private:
			std::unordered_map<std::string,WriteStream> Streams;
			WriteStream * MostRecentStream = nullptr;
			WriteStream * LargeFile = nullptr;
			std::ofstream OutputWriter;
			internal::Security Security;
			std::string TempName;
			std::streampos LargeFileHeaderPos;
		public:
			//! Default initialiser, which places the object into an Uninitialised state. 
			Vault();

			//! Initialises the object and immediately calls Open() @param archivePath The name of the archive on disk @param mode The ::Mode assigned to the object
			Vault(const std::string & archivePath, bool strictMode = false) ;

			~Vault () noexcept;

			void Open(std::string_view path);
			
			void Close();
			WriteStream & operator[](const std::string & streamName);
			
			template<class T>
			Vault & operator<<(T msg)
			{
				if (!MostRecentStream)
				{
					JSL::internal::FatalError("Attempting to write to an archive before a write stream has been opened");
				}
				MostRecentStream->Feed(msg);
				return *this;
			}

			void SetWritePermissions(uint32_t uid=1000, uint32_t gid=1000, std::string uname="user", std::string gname="user");
			void SetLargeFile(const std::string & file);
		   
	};

};
