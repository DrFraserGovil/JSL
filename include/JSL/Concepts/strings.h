#pragma once
#include <concepts>
#include <string_view>
#include <string>
#include <chrono>
#include <memory>
namespace JSL::Concept
{
	template<typename T>
	concept StringType = std::convertible_to<T,std::string>;

}