
#pragma once
#include <JSL/Strings/Templates/make_tuple.h>
#include <JSL/Strings/MakeFrom.h>
namespace JSL::String
{
	/*! @brief Converts an iterable range into a string, recursively converting the contained objects
		@details The string has bracket endcaps ("[]") and a comma delimiter. For more fine grained control, see JSL::String::stitch
	 * @tparam T An object supporting range-based iteration (but not strings)
	 * @param obj The object to be converted
	 * @return A string representing the input object 
	 */
	template<JSL::Concept::NonStringRange T>
	std::string inline makeFrom(const T & obj)
	{
		std::ostringstream os;
 		os << "[";
		bool first = true;
		for (const auto & v : obj)
		{
			if (!first)
			{
				os << ", ";
			}
			first = false;
			os << makeFrom(v);
		}
		os << "]";
		return os.str();
	}
}