#include "base.h"
#include "helpers.h"
#include <unordered_map>
#include <algorithm>

#include <cstring>
#include <vector>
#include <functional>
#include <string_view>
#include "readStream.h"
namespace JSL::Archiver
{
    //templte specialisation for the vault-writer
    template<>
    class Vault<Mode::Read> : public internal::VaultBase
    {
        private:
            std::ifstream FileStream;
            std::unordered_map<std::string,ReadStream> FileIndex;
            bool ReadBlock(char* buffer)
            {
                
                FileStream.read(buffer, internal::BLOCK_SIZE);
                return FileStream.gcount() == internal::BLOCK_SIZE;
            }
            
            void BuildIndex()
            {
                //This completeing without throwing is part of the validation efforts!
                char header[internal::BLOCK_SIZE];
                int zeroBlockCount = 0;
                while (ReadBlock(header)) 
                {
                    // Check for all-zero block (end of archive is two such blocks)
                    if (std::all_of(header, header + internal::BLOCK_SIZE, [](char c) { return c == '\0'; })) 
                    {
                        ++zeroBlockCount;
                        if (zeroBlockCount >= 2)
                        {
                            break;
                        }
                    }	
                    else
                    {
                        zeroBlockCount = 0; //reset the count -- termination sequence is two contiguous null blocks
                    

                        // Parse metadata
                        auto name  = std::string(header, 100);
                        name.erase(name.find('\0')); // Remove null padding

                        char size_str[12];
                        std::memcpy(size_str, header + 124, 12); // File size starts at offset 124
                        size_str[11] = '\0'; // Ensure null-termination
                        size_t size = std::strtol(size_str,nullptr,8);

                        std::streampos data_offset = FileStream.tellg(); // Current position after reading the header

                        FileIndex.try_emplace(name,ReadStream(size,data_offset,FileStream));

                        // Skip file data and padding
                        size_t data_blocks = (size + internal::BLOCK_SIZE - 1) / internal::BLOCK_SIZE;
                        FileStream.seekg(data_blocks * internal::BLOCK_SIZE, std::ios::cur);
                    }
                }

                if (zeroBlockCount != 2)
                {
                    JSL::internal::FatalError("Archive Scan did not locate null termination sequence. The Archive " + Name + " is corrupted or incomplete");
                }
            }
        public:
            Vault(): internal::VaultBase(Mode::Read)
            {
                Initialised = false;
            }

            //! Initialises the object and immediately calls Open() @param archivePath The name of the archive on disk @param mode The ::Mode assigned to the object
            Vault(const std::string & archivePath, bool strictMode = false)  : internal::VaultBase(Mode::Read)
            {
                StrictMode = strictMode;
                Open(archivePath);
            }
            ~Vault() noexcept
            {
                Close();
            }
            void Open(std::string_view path)
            {
                //if initialised, close out the old stuff
                if (Initialised)
                {
                    Close();
                }
                Name = (std::string) path;
                FileStream.open(Name,std::ios::in|std::ios::binary);
                if (!FileStream.is_open())
                {
                    JSL::internal::FatalError("Could not open archive file " + Name);
                }
                BuildIndex();
                Initialised = true;
            }

            void Close()
            {
                //Closing doesn't really do much beside shut off the file streams
                FileStream.close();
            }

            std::vector<std::string> ListFiles()
            {
                std::vector<std::string> filenames;

                for (auto file : FileIndex)
                {
                    filenames.push_back(file.first);
                }
                return filenames;
            }

            ReadStream & operator[](const std::string & file)
            {
                auto it = FileIndex.find(file);
                if (it == FileIndex.end())
                {
                    JSL::internal::FatalError("File `" + file + "` is not in archive " + Name);
                }
                
                return it->second;
            }

            void ForLineIn(const std::string & file,std::function<void(std::string_view)> perLineFunction)
            {
                (*this).operator[](file).ForLineIn(perLineFunction);
            }

            template<typename...ColumnTypes, typename TupleFunctor>
            void ForTabularLineIn(const std::string & file,std::string delimiter,TupleFunctor perTupleFunction)
            {
                (*this).operator[](file).ForTabularLineIn<ColumnTypes...>(delimiter,perTupleFunction);
            }
    };

}