#pragma once
#include <memory>
namespace JSL::Concept
{
	namespace internal
	{
		template <typename T>
		struct is_unique_ptr : std::false_type
		{
		};
		template <typename... Args>
		struct is_unique_ptr<std::unique_ptr<Args...>> : std::true_type
		{
		};

		template <typename T>
		struct is_shared_ptr : std::false_type
		{
		};
		template <typename... Args>
		struct is_shared_ptr<std::shared_ptr<Args...>> : std::true_type
		{
		};
	} // namespace internal

	//! @brief  Matches all types which are wrapped in a unique pointer
	template <typename T>
	concept UniquePtr = internal::is_unique_ptr<T>::value;

	//! @brief  Matches all types which are wrapped in a shared pointer
	template <typename T>
	concept SharedPtr = internal::is_shared_ptr<T>::value;

	//! @brief Matches all types which are either a shared_poiter or a unique_pointer
	template <typename T>
	concept SmartPtr = UniquePtr<T> || SharedPtr<T>;
} // namespace JSL::Concept
