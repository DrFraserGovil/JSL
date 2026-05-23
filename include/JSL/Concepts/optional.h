#pragma once
#include <concepts>
#include <optional>
namespace JSL::Concept
{
	namespace internal
	{
		template<typename T> struct is_optional : std::false_type {};
		template<typename...Args> struct is_optional<std::optional<Args...>> : std::true_type  {};
	}

	template<typename T> concept OptionalLike = internal::is_optional<T>::value;
}