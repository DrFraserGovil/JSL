
#pragma once
#include <JSL/Strings/ParseTo.h>
#include <JSL/Concepts/pointers.h>
#include <memory>
namespace JSL::String
{
	template<JSL::Concept::UniquePtr T>
	T ParseTo(std::string_view sv)
	{
		using type = typename T::element_type;
		return std::make_unique<type>(std::move(ParseTo<type>(sv)));
	}
	template<JSL::Concept::SharedPtr T>
	T ParseTo(std::string_view sv)
	{
		using type = typename T::element_type;
		return std::make_shared<type>(std::move(ParseTo<type>(sv)));
	}
}