#pragma once
#include <optional>
namespace JSL::Concept
{
	namespace internal
	{
		template <typename T>
		struct is_optional : std::false_type
		{
		};
		template <typename... Args>
		struct is_optional<std::optional<Args...>> : std::true_type
		{
		};
	} // namespace internal

	//! A Concept which matches with any std::optional<T>
	template <typename T>
	concept OptionalLike = internal::is_optional<T>::value;
} // namespace JSL::Concept
