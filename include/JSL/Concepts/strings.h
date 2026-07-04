#pragma once
#include <concepts>
#include <string>
#include <string_view>
namespace JSL::Concept
{
	//! A concept that matches any string, and the associated subtypes (char *, std::string_view, and so on)
	template <typename T>
	concept StringLike = std::convertible_to<T, std::string> || std::convertible_to<T, std::string_view>;

} // namespace JSL::Concept
