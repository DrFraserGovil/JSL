#pragma once
#include <JSL/Strings/ParseTo.h>
#include <JSL/Concepts.h>
#include <optional>
namespace JSL::String
{
	template<JSL::Concept::OptionalLike T>
	T ParseTo(std::string_view sv)
	{
		sv=trim_view(sv);
		//no reject empty as we allow an empty signal to also be a failure

		if (sv.empty() || sv == "__none__")
		{
			return std::nullopt;
		}

		return ParseTo<typename T::value_type>(sv);
	}
}