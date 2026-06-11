#include <JSL/FileIO/archiver/writer.h>
#include <JSL/internal/error.h>
#include <filesystem>
#include <JSL/Log.h>
namespace JSL::Archiver
{
	Vault<Mode::Write>::Vault() : internal::VaultBase(Mode::Write)
	{
		Initialised = false;
	}

	Vault<Mode::Write>::Vault(const std::string & archivePath, bool strictMode)  : internal::VaultBase(Mode::Write)
	{
		StrictMode = strictMode;
		Open(archivePath);
	}

	Vault<Mode::Write>::~Vault () noexcept
	{
		if (Initialised)
		{
			try
			{
				Close();
			}
			catch (...)
			{
				LOG(WARN) << "There was a fatal error in archive destructor -- data not written to file";
			}
		}
	}
	void Vault<Mode::Write>::Open(std::string_view path)
	{
		//if initialised, close out the old stuff
		if (Initialised)
		{
			Close();
		}
			//initialise & clear existing structures
		Name = (std::string)path;
		TempName = Name + ".part";

		//check for name collisions if being careful
		if (StrictMode)
		{
			bool fileExists = std::filesystem::exists(Name);
			if (fileExists)
			{
				JSL::internal::FatalError("StrictMode interrupt: a file with the name `" + Name + "` exists at that location.\nChange the file names, or disable StrictMode if you wish to overwrite the file", JSL_LOCATION) ;
			}
		}

		OutputWriter.open(TempName, std::ios::out | std::ios::binary);
		if (!OutputWriter.is_open())
		{
			JSL::internal::FatalError("Failed to open archive " + TempName + " in write mode", JSL_LOCATION) ;
		}
		Initialised = true;
		MostRecentStream = nullptr;
		LargeFile = nullptr;
		
	}

	void Vault<Mode::Write>::Close()
	{
		if (!Initialised)
		{
			return;
		}

		//do the large streams first
		if (LargeFile)
		{
			auto pkg = LargeFile->Export(Security);
			OutputWriter.seekp(LargeFileHeaderPos);
			OutputWriter.write(reinterpret_cast<const char*>(&pkg.headerBlock), sizeof(pkg.headerBlock));

			OutputWriter.seekp(0,std::ios::end);
				if (pkg.padding > 0)
			{
				OutputWriter.write(internal::zeroBuffer, pkg.padding);
			}
		}

		//iterate over the streams
		for (auto & [name,stream] : Streams)
		{
			if (stream.NeedsWriting())
			{
				auto pkg = stream.Export(Security);
				OutputWriter.write(reinterpret_cast<const char*>(&pkg.headerBlock), sizeof(pkg.headerBlock));
				OutputWriter.write(pkg.data.data(), pkg.data.size());
				
				if (pkg.padding > 0)
				{
					OutputWriter.write(internal::zeroBuffer, pkg.padding);
				}
			}
		}
		
		//Write end-of-archive sequence - a set of two 0-blocks
		OutputWriter.write(internal::END_OF_ARCHIVE,2*internal::BLOCK_SIZE);
		
		//cleanup
		OutputWriter.close();
		Streams.clear();
		Initialised = false;
		MostRecentStream = nullptr;
		LargeFile = nullptr;

		//perform the renaming
		std::filesystem::rename(TempName,Name);
	}
	
	WriteStream & Vault<Mode::Write>::operator[](const std::string & streamName)
	{
		if (!Initialised)
		{
			JSL::internal::FatalError("Cannot get streams until the archive has been opened", JSL_LOCATION) ;
		}
		auto it = Streams.find(streamName);
		if (it == Streams.end())
		{
			if (StrictMode)
			{
				JSL::internal::FatalError("Whilst in strict mode, a vault attempted to write to a non-existent file: " + streamName, JSL_LOCATION) ;
			}
			it = Streams.try_emplace(streamName, streamName).first;
		}
		
		MostRecentStream = &it->second;
		return *MostRecentStream;

	}

void Vault<Mode::Write>::SetWritePermissions(uint32_t uid, uint32_t gid, std::string uname, std::string gname)
	{
		Security.UID = uid;
		Security.GID = gid;
		Security.UName = std::move(uname);
		Security.GName = std::move(gname);
	}

	void Vault<Mode::Write>::SetLargeFile(const std::string & file)
	{
		auto & target = this->operator[](file);
		LargeFile = &target;
		LargeFileHeaderPos = OutputWriter.tellp();
		target.SetLarge(OutputWriter);
	}
};
