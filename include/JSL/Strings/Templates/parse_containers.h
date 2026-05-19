#pragma once
#include <algorithm>
#include <string_view>
#include <JSL/Concepts.h>
#include <JSL/Strings/Templates/helpers.h>
#include <JSL/Strings/ParseTo.h>
namespace JSL::String
{
	//forward declaration
	template<JSL::Concept::NonStringRange T>
	T ParseTo(std::string_view sv);

	template<JSL::Concept::NonStringRange T>
	T inline ParseTo(std::string_view sv,std::string_view delimiter)
	{
		using InnerT = std::ranges::range_value_t<T>;
		std::vector<std::string_view> tokens;
		
		
		T out{};
		if constexpr (JSL::Concept::NonStringRange<InnerT> || JSL::Concept::TupleLike<InnerT>)
		{
			tokens = (internal::recursetokens(sv));
			if (tokens.empty() && !sv.empty())
			{
				JSL::internal::FatalError("Bad container parse",JSL_LOCATION) << "The string " << sv << " is non-empty, but no matching container boundaries can be found";
			}
		}
		else
		{ 
			tokens = (internal::tokenize(sv,delimiter,typeid(T).name()));	
		}
		if (tokens.empty())
		{
			return out;
		}		


		std::transform(tokens.begin(),tokens.end(),std::inserter(out,out.end()),
			[&](const auto & token)
			{
				if constexpr (JSL::Concept::NonStringRange<InnerT> || JSL::Concept::TupleLike<InnerT>)
				{
					return JSL::String::ParseTo<InnerT>(token, delimiter);
				}
				else
				{
					return JSL::String::ParseTo<InnerT>(token);
				}
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