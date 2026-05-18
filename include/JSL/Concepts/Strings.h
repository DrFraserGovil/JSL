#pragma once
#include <concepts>
#include <string_view>
#include <string>
namespace JSL::Concept
{
	template<typename T>
	concept StringType = std::convertible_to<T,std::string>;


}