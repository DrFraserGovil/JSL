#pragma once
#include <functional>
#include <string_view>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include <JSL/Strings/Manipulate.h>
#include "helpers.h"
namespace JSL::Archiver{

	class ReadStream
	{
		private:
			size_t Size;
			std::streampos data_offset; //!< Byte offset to the file's data in the archive
			std::ifstream & StreamCopy; //!< A copy of the stream so it can read by itself

			void StreamBlocks(std::function<void(std::string_view)> callback);

			
		public:
			ReadStream(size_t size, std::streampos offset, std::ifstream & stream);
			
			void ForLineIn(std::function<void(std::string_view)> perLineFunction);
			
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
					
					auto row = ParseTo<ColumnTypes...>(String::split_view(line,delimiter));

					perTupleFunction(row);

				});
			}
			std::string AsText();
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