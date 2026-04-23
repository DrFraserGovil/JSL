#pragma once
#include "../../Strings/ParseTo.h"
#include "base.h"
#include "helpers.h"
#include <sstream>
#include <algorithm>
#include <charconv>
#include <fstream>
#include <functional>
#include <string_view>
namespace JSL::Archiver{

    class ReadStream
    {
        private:
            size_t Size;
            std::streampos data_offset; //!< Byte offset to the file's data in the archive
            std::ifstream & StreamCopy; //!< A copy of the stream so it can read by itself

            void StreamBlocks(std::function<void(std::string_view)> callback)
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
        public:
            ReadStream(size_t size, std::streampos offset, std::ifstream & stream): StreamCopy(stream)
            {
                // Name = std::move(name);
                Size = size;
                data_offset = offset;
            }

            
            void ForLineIn(std::function<void(std::string_view)> perLineFunction)
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

            /*!
					@brief Parse the datafile and convert each line into a tuple of datatypes based on an assumed regular tabular. Then performs a callback function on each line.

					@details Each line is converted into a tuple of types specified by the ColumnTypes parameter (i.e. ForTabularLineIn<int,int,int> attempts to read the file as a set of three integers). Tuples are then accessed via std::get<i> to get the ith element of the tuple.
					@param ColumnTypes An (arbitrary) number of typenames, representing the converted types of each element. typenames must have an associated convert() function.
					@param TupleFunctor (optional -- usually compiler-inferred) template parameter for the type of the per-tuple callback function
					@param delimiter The string delimiter between ColumnTypes in the file
					@param perTupleFunction The function to be called on 

					@throws runtime_error If the columns of the file cannot be interpreted as a regular grid of len(ColumnTypes) with consistent data types with the specified delimiter
					@throws runtime_error If a datatype in a column cannot be converted into the specified typr
					@throws logic_error when called whilst in write mode 
					@throws logic_error when fileName not in the archive
					
				*/
				template<typename...ColumnTypes, typename TupleFunctor>
				void ForTabularLineIn(std::string delimiter,TupleFunctor perTupleFunction)
				{
					ForLineIn([&](std::string_view line){
						
						auto row = ParseTo<ColumnTypes...>(split(line,delimiter));

						perTupleFunction(row);

					});
				}
                std::string AsText()
            {
                std::string buffer = "";
                StreamBlocks([&](auto block){
                    buffer += std::string(block);
                });
                return buffer;
            }

            template<typename...ColumnTypes>
            std::vector<std::tuple<ColumnTypes...>> AsTable(const std::string & delimiter =" ")
            {
                std::vector<std::tuple<ColumnTypes...>> rows;
					ForTabularLineIn<ColumnTypes...>(delimiter,[&](auto row)
					{
						rows.push_back(row);
					});
					return rows;
            }


    };


}