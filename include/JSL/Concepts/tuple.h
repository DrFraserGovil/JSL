#pragma once
#include <tuple>
namespace JSL::Concept
{
	//! @brief A template concept which matches to both std::tuple and std::pair, and anything else with a similar interface
	template <typename T>
	concept TupleLike = requires(T t) {
		std::tuple_size<T>::value;
		std::apply([](auto &&...) {}, t);
	};
} // namespace JSL::Concept
