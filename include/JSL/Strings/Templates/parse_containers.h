#pragma once
#include <algorithm>
#include <string_view>
#include <JSL/Concepts.h>
#include <JSL/Strings/Templates/helpers.h>
#include <JSL/Strings/ParseTo.h>
namespace JSL::String
{
	template<JSL::Concept::NonStringRange T>
	T inline ParseTo(std::string_view sv,std::string_view delimiter)
	{
		auto tokens = internal::tokenize(sv,delimiter,typeid(T).name());	

		T out{};
		if (tokens.empty())
		{
			return out;
		}		

		std::transform(tokens.begin(),tokens.end(),std::inserter(out,out.end()),
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