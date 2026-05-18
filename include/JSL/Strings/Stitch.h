#pragma once
#include <string>
#include <vector>
#include <JSL/Strings/MakeFrom.h>
#include <sstream>
#include <JSL/Concepts.h>
namespace JSL::String
{
	/*!
	 * @brief Stitch together the contents of a vector into a string separated by a custom delimiter
	 * @tparam container A container which supports ranges, size() and array subscripting
	 * @param vec The input container
	 * @param begin The first index to be included in the output 
	 * @param end The index at which inclusion terminates. Does not appear in the final output
	 * @param delim The string which separates entries
	 * @return A string representation of a subset of the input vector
	 */
	template<JSL::Concept::IndexableRange container>
	std::string stitch(const container & vec, size_t begin, size_t end, std::string_view delim)
	{
		if (begin >= end || begin >= vec.size())
		{
			return "";
		}
		std::ostringstream os;
		
		os << represent(vec[begin]);
		for (size_t i = begin + 1; i < end && i < vec.size(); ++i)
		{
			os << delim  << represent(vec[i]);
		}
		return os.str();
	}

	  
	/*!
	 * @brief Stitch together the contents of a vector into a string separated by a custom delimiter
	 * @tparam container A container which supports ranges, size()
	 * @param vec The input container
	 * @param delim The string which separates entries
	 * @return A string representation of the input vector
	 */
	template<JSL::Concept::SearchableRange container>
	std::string stitch(const container & vec, std::string_view delim)
	{
		std::ostringstream os;
		
		bool first = true;
		for (auto & v : vec)
		{
			if (!first)
			{
				os << delim;
			}
			else
			{
				first = false;
			}
			os << represent(v);
		}
		return os.str();
	}
	 
}