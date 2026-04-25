#include <JSL/FileIO/archiver/readStream.h>

#include <sstream>
#include <algorithm>
#include <charconv>
#include <fstream>
#include <stdio.h>

namespace JSL::Archiver{


    void ReadStream::StreamBlocks(std::function<void(std::string_view)> callback)
    {
        const unsigned int blocksInBuffer = 10; //each block is 0.5kb, so this means we're reading up to 5kb at a time, an eminently reasonable buffer size 


        StreamCopy.seekg(data_offset);
        size_t remainingSize = Size;
        char buffer[blocksInBuffer*internal::BLOCK_SIZE];

        while (remainingSize > 0)
        {
            size_t chunk_size = std::min(remainingSize, sizeof(buffer));
            StreamCopy.read(buffer, chunk_size);
            callback(std::string_view(buffer,buffer+chunk_size));
            remainingSize -= chunk_size;
        }
    }
    
    ReadStream::ReadStream(size_t size, std::streampos offset, std::ifstream & stream): StreamCopy(stream)
    {
        // Name = std::move(name);
        Size = size;
        data_offset = offset;
    }

    
    void ReadStream::ForLineIn(std::function<void(std::string_view)> perLineFunction)
    {
        //have to be clever because StreamBlocks (and the tar protocol in general) does not guarantee blocks are complete lines
        std::string overflow; //entity for holding overflow from the previous block
        StreamBlocks([&](std::string_view block) 
        {
            size_t current_pos = 0;
            
            // If we had overflow, complete that line first
            if (!overflow.empty())
            {
                size_t next_nl = block.find('\n');
                if (next_nl != std::string_view::npos)
                {
                    overflow.append(block.data(), next_nl);
                    perLineFunction(overflow);
                    overflow.clear();
                    current_pos = next_nl + 1;
                }
                else
                {
                    overflow.append(block.data(), block.size());
                    return; // Entire block was just more of the same line
                }
            }

            // Process all complete lines in the current block
            while (true)
            {
                size_t next_nl = block.find('\n', current_pos);
                if (next_nl == std::string_view::npos) break;

                perLineFunction(block.substr(current_pos, next_nl - current_pos));
                current_pos = next_nl + 1;
            }

            // Save remaining partial line to overflow
            if (current_pos < block.size())
            {
                overflow.assign(block.data() + current_pos, block.size() - current_pos);
            }
        });

        if (!overflow.empty()) perLineFunction(overflow);

    }


    std::string ReadStream::AsText()
    {
        std::string buffer = "";
        StreamBlocks([&](auto block){
            buffer += std::string(block);
        });
        return buffer;
    }

};

