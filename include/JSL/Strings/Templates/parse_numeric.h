#pragma once
#include <JSL/Strings/Templates/helpers.h>
#include <JSL/Concepts.h>
#include <charconv>
namespace JSL::String
{
	/*! @brief Parses strings into numeric types 
	 	@warning Uses std::to_chars, which may cause some issues with non-C++20 compliant compilers (Apple Clang, for example), for which this function is not implemented for doubles 
		@tparam T A type which can be converted into a string 
		@param sv A string to be parsed 
		@return An object of type T represented by the input string
	 */
	template<JSL::Concept::Numeric T>
	T inline ParseTo(std::string_view sv)
	{
		sv=trim_view(sv);
		internal::RejectEmpty(sv,typeid(T).name());
		T output{};
		auto result = std::from_chars(sv.data(),sv.data() + sv.size(), output);
		internal::CheckErrors(result,sv,typeid(T).name());

		return output;
	} 
}