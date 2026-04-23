#pragma once
#include "base.h"
#include "helpers.h"
#include <sstream>
#include <algorithm>
#include <charconv>
#include <fstream>
namespace JSL::Archiver{
    template<Mode T>
    class Vault; //forward declaration for friend to latch onto

    class WriteStream
    {
        private:
            std::unique_ptr<std::stringstream> MemoryBuffer;
            std::ostream* ActiveStream;
            std::string Name;
            template <Mode T>
            friend class Vault;
            internal::WriteExporter Export(const internal::Security & security)
            {
                internal::WriteExporter pkg;
                

                auto & header = pkg.headerBlock;

                auto name_size = std::min(Name.size(), header.name.size() - 1);
                std::copy_n(Name.begin(), name_size, header.name.data());

                // Helper for Octal Conversion via std::to_chars
                auto to_octal = [](auto& arr, uint64_t value) 
                {
                    // TAR octal fields are usually N-1 chars + null/space: fill with '0'.
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
                };

                size_t writeSize;
                if (IsLarge)
                {
                    pkg.data = "";
                    writeSize = LargeWriteCount;
                }
                else
                {
                    pkg.data = MemoryBuffer->str();
                    MemoryBuffer->str("");
                    MemoryBuffer->clear();
                    writeSize = pkg.data.size();
                }
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


                pkg.padding = (512u - (writeSize % 512u)) & 511u;

                return pkg;
            };
            bool IsLarge=false;
            size_t LargeWriteCount = 0;
            void SetLarge(std::ostream & diskStream){
                IsLarge = true;
                
                ActiveStream = &diskStream;
                
                internal::TarHeader dummy{};
                ActiveStream->write(reinterpret_cast<const char*>(&dummy), 512);

                //then feed what's currently in the memory buffer into the active stream
                auto existing = MemoryBuffer->str();
                (*ActiveStream) << existing;
                LargeWriteCount = existing.size();
                MemoryBuffer.reset();
            };
            template<class T>
            std::ostream&Feed(const T & msg)
            {
                if (IsLarge)
                {
                    auto start = ActiveStream->tellp();
                    (*ActiveStream) << msg;
                    LargeWriteCount += (ActiveStream->tellp() - start);
                }
                else {  (*ActiveStream) << msg;  }
                
                return (*ActiveStream);
            }
            bool NeedsWriting()
            {
                return !IsLarge;
            }

        public:
            
            WriteStream(std::string_view name) : Name(std::string(name)){
                MemoryBuffer = std::make_unique<std::stringstream>();
                ActiveStream = MemoryBuffer.get();
            };
            
            template<class T>
            std::ostream&operator<<(const T & msg)
            {
                return Feed(msg);
            }

           
            
    };
} //end namespace