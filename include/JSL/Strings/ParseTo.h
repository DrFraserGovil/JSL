#pragma once
#include <iostream>
#include <string_view>
#include <vector>
#include <charconv>
#include "Manipulate.h"
#include "Cases.h"
#include <JSL/internal/error.h>
#include <JSL/Concepts.h>
#include <algorithm>

#include <JSL/Strings/MakeFrom.h>
namespace JSL::String
{
	namespace internal
	{
		//helper functions 
		void CheckErrors(std::from_chars_result & result,std::string_view sv,std::string_view typeName);
		void RejectEmpty(std::string_view sv,std::string_view typeName);
		std::string_view StripEndCaps(std::string_view sv);
	}



	//////////////////////
	// DEFAULT TEMPLATES
	//////////////////////


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

	//The apple-clang override for doubles
	#if defined(__clang__) && defined(__APPLE__)
		template<>
		double ParseTo(std::string_view sv);
	#endif

	template<>
	char ParseTo(std::string_view sv);
	
	template<>
	bool ParseTo(std::string_view sv);


	/////////////
	// VECTORS
	////////////
	
	
	template<JSL::Concept::NonStringRange T>
	T inline ParseTo(std::string_view sv,std::string_view delimiter)
	{
		sv=trim_view(sv);
		internal::RejectEmpty(sv,typeid(T).name());
		//! We allow vectors to be wrapped in either [], {} or (). This function removes them for internal use.
		sv = internal::StripEndCaps(sv);

		if (sv.empty())
		{
			return T{}; //only reach here if we were given a set of blank end caps (i.e. vec = []), so we return it as empty
		}

		auto tokens = split_view(sv,delimiter);

		auto trimmed = tokens | std::views::filter([](auto t){ return !(trim_view(t)).empty();});

		T out{};
		
		std::transform(trimmed.begin(),trimmed.end(),std::inserter(out,out.end()),
			[&](const auto & token)
			{
				return JSL::String::ParseTo<std::ranges::range_value_t<T>>(token); 
			}	
		);
		return out;
	}
	
	template<JSL::Concept::NonStringRange T>
	T inline ParseTo(std::string_view sv)
	{
		return ParseTo<T>(sv,",");
	}
}
