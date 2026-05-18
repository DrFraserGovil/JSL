#pragma once
#include <JSL/Strings/Templates/parse_generic.h>
namespace JSL::String
{
	//The apple-clang override for doubles
	#if defined(__clang__) && defined(__APPLE__)
		template<>
		double ParseTo(std::string_view sv);
	#endif

	template<>
	char ParseTo(std::string_view sv);
	
	template<>
	bool ParseTo(std::string_view sv);

}