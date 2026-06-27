#pragma once
#include <concepts>
#include <string_view>
#include <vector>
namespace JSL::Concept
{

	// A custom lightweight type-trait alias to replace std::ranges::range_value_t
	template <typename T>
	using RangeInternalType = std::iter_value_t<decltype(std::begin(std::declval<T &>()))>;

	// Core structural check to verify a type supports basic iteration
	template <typename T>
	concept Iterable = requires(T &t) {
		std::begin(t);
		std::end(t);
	};

	// A concept which defines objects that can be iterated through, and has a 'size' member
	template <typename T>
	concept SearchableRange = Iterable<T> &&
							  requires(T t) {
								  { t.size() } -> std::integral;
							  };

	template <typename T>
	concept IndexableRange = SearchableRange<T> &&
							 requires(T t, size_t index) {
								 t[index];
							 };

	template <typename T>
	concept NonStringRange = Iterable<T> &&
							 !std::convertible_to<T, std::string_view> &&
							 !std::convertible_to<T, std::string>;

} // namespace JSL::Concept
