#pragma once
#include "base.h"
#include "helpers.h"
#include <sstream>
#include <algorithm>
#include <charconv>
#include <fstream>
#include <memory>
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
            internal::WriteExporter Export(const internal::Security & security);
            
            bool IsLarge=false;
            size_t LargeWriteCount = 0;
            void SetLarge(std::ostream & diskStream);
        
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
            bool NeedsWriting();

        public:
            
            WriteStream(std::string_view name);
            
            template<class T>
            std::ostream&operator<<(const T & msg)
            {
                return Feed(msg);
            }

           
            
    };
} //end namespace