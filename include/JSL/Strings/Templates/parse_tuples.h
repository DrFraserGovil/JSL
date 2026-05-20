#pragma once
#include <JSL/Concepts.h>
#include <JSL/Strings/Templates/stringparsers.h>
#include <JSL/Strings/ParseTo.h>
namespace JSL::String
{
	namespace internal
	{
		template<JSL::Concept::TupleLike T, std::size_t... Is>
		T ParseToTupleImpl(const std::vector<std::string_view>& tokens, std::index_sequence<Is...>)
		{
			return T{ JSL::String::ParseTo<std::tuple_element_t<Is, T>>(tokens[Is])... };
		}
	}

	template<JSL::Concept::TupleLike T>
	T ParseTo(std::string_view sv,std::string_view delim)
	{
		auto tokens = internal::tokenize(sv,delim,typeid(T).name());

		if (tokens.size() != std::tuple_size_v<T>)
		{
			JSL::internal::FatalError("Tuple parse error", JSL_LOCATION)
				<< "Expected " << std::tuple_size_v<T> << " elements, got " << tokens.size();
		}

		return internal::ParseToTupleImpl<T>(tokens,std::make_index_sequence<std::tuple_size_v<T>>{});
	}
	template<JSL::Concept::TupleLike T>
	T ParseTo(std::string_view sv)
	{
		return ParseTo<T>(sv,",");
	}	

}