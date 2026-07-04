#pragma once
#include <JSL/Concepts/strings.h>
#include <concepts>
#include <string_view>
#include <vector>
namespace JSL::Concept
{

	//! @brief A concept which extracts the 'internal type' of range-like objects; useful for writing template functions which accept arbitrary ranges as inputs
	//! @details A functional duplicate of ``std::ranges::range_value_t`` that does not require transcluding the (somewhat heavy) std::ranges header
	template <typename T>
	using RangeInternalType = std::iter_value_t<decltype(std::begin(std::declval<T &>()))>;

	//! A concept matching types which support basic iteration
	//! @details This is a slightly primitive version (the advanced one would use std::ranges); but this suffices in 99% of cases, without requiring us to include the ranges header
	template <typename T>
	concept Iterable = requires(T &t) {
		std::begin(t);
		std::end(t);
	};

	//! A concept which defines objects that can be iterated through, and has a 'size' member
	template <typename T>
	concept SearchableRange = Iterable<T> &&
							  requires(T t) {
								  { t.size() } -> std::integral;
							  };
	//! A concept which mataches any SearchableRange which can also be indexed using [] notation
	template <typename T>
	concept IndexableRange = SearchableRange<T> &&
							 requires(T t, size_t index) {
								 t[index];
							 };

	//! A concept which matches any iterable range, but which is **not** an std::string (or related subtypes)
	//! @details This is useful because strings ofen try to convert themselves into their underlying vectorish type at inopportune moments.
	template <typename T>
	concept NonStringRange = Iterable<T> &&
							 !StringLike<T>;

} // namespace JSL::Concept
