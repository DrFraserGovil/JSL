#include "base.h"
#include "writeStream.h"
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
            Vault() : internal::VaultBase(Mode::Write)
            {
                Initialised = false;
            }

            //! Initialises the object and immediately calls Open() @param archivePath The name of the archive on disk @param mode The ::Mode assigned to the object
            Vault(const std::string & archivePath, bool strictMode = false)  : internal::VaultBase(Mode::Write)
            {
                StrictMode = strictMode;
                Open(archivePath);
            }

            ~Vault () noexcept
            {
                if (Initialised)
                {
                    try
                    {
                        Close();
                    }
                    catch (...)
                    {
                        std::cout << "There was a fatal error in archive destructor -- data not written to file";
                    }
                }
            }

            void Open(std::string_view path)
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
                        JSL::internal::FatalError("StrictMode interrupt: a file with the name `" + Name + "` exists at that location.\nChange the file names, or disable StrictMode if you wish to overwrite the file");
                    }
                }

                OutputWriter.open(TempName, std::ios::out | std::ios::binary);
                if (!OutputWriter.is_open())
                {
                    JSL::internal::FatalError("Failed to open archive " + TempName + " in write mode");
                }
                Initialised = true;
                MostRecentStream = nullptr;
                LargeFile = nullptr;
               
            }

            void Close()
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
            WriteStream & operator[](const std::string & streamName)
            {
                if (!Initialised)
                {
                    JSL::internal::FatalError("Cannot get streams until the archive has been opened");
                }
                auto it = Streams.find(streamName);
                if (it == Streams.end())
                {
                    if (StrictMode)
                    {
                        JSL::internal::FatalError("Whilst in strict mode, a vault attempted to write to a non-existent file: " + streamName);
                    }
                    it = Streams.try_emplace(streamName, streamName).first;
                }
                
                MostRecentStream = &it->second;
                return *MostRecentStream;

            }
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

            void SetWritePermissions(uint32_t uid=1000, uint32_t gid=1000, std::string uname="user", std::string gname="user")
            {
                Security.UID = uid;
                Security.GID = gid;
                Security.UName = std::move(uname);
                Security.GName = std::move(gname);
            }

            void SetLargeFile(const std::string & file)
            {
                auto & target = this->operator[](file);
                LargeFile = &target;
                LargeFileHeaderPos = OutputWriter.tellp();
                target.SetLarge(OutputWriter);
            }
           
    };

};
