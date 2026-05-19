#pragma once
#include <JSL/Strings/Templates/helpers.h>
#include <JSL/Strings/Manipulate.h>
#include <JSL/internal/error.h>
namespace JSL::String
{
	
	/*! @brief The default template for parsing a string into a generic type
		@details Attempts a naive string-native construction. We don't require that such a construction exists as this is also our gateway to error messages
		@tparam T A type which can be converted into a string 
		@param sv A string to be parsed 
		@return An object of type T represented by the input string
	 */
	template<class T>
	T inline ParseTo(std::string_view sv)
	{
		sv=trim_view(sv);
		internal::RejectEmpty(sv,typeid(T).name());
		try 
		{
			return T{sv};
		}
		catch (...)
		{
		
			JSL::internal::FatalError("Bad conversion",JSL_LOCATION) << "Cannot convert " << sv << " to type " << typeid(T).name() << " using the default converter.";
			return ""; //dead code, suppresses compiler warnings
		}
	}
} // namespace JSL::String
