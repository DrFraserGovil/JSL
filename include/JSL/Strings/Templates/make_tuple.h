#pragma once
#include <JSL/Strings/MakeFrom.h>
namespace JSL::String
{
	/*! @brief Converts a tuple into a string, recursively converting the contained objects	
		@details The string has bracket endcaps ("()") and a comma delimiter. 
		@tparam T Any std::tuple or std::pair object 
		@param obj The value to be stringified
		@return A string representing the input value
		@throws std::runtime_error: If the value cannot be converted to a string
	*/
	template<JSL::Concept::TupleLike T>
	std::string inline makeFrom(const T & obj)
	{
		std::ostringstream os;
		os << "(";
		std::apply([&os, first = true](const auto&... args) mutable
		{
			((os << (first ? first = false, "" : ", ") << makeFrom(args)), ...);
		}, obj);
		os << ")";
		return os.str();
	}
}