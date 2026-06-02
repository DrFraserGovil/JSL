#pragma once
#include <tuple>
namespace JSL::Concept
{
	template<typename T>
	concept TupleLike = requires(T t)
	{
		std::tuple_size<T>::value;
		std::apply([](auto&&...){}, t);
	};
}
