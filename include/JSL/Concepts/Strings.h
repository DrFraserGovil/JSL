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

	// namespace internal
	// {
	// 	template<typename T>
	// 	struct is_optional : std::false_type {};
		
	// 	template<typename T>
	// 	struct is_optional<std::optional<T>> : std::true_type {};

	// }

	#define WrapperConcept(name, wrapperType, storage)                                    \
		namespace internal {                                                              \
			template<typename T> struct storage : std::false_type {};                     \
			template<typename... Args> struct storage<wrapperType<Args...>> : std::true_type {}; \
		}                                                                                 \
		template <typename T> concept name = internal::storage<T>::value;
	//enddefine
	

	#ifndef JSL_NULL_STRING
		//! The string value used to indicate that a string represents an std::optional or a smart pointer that does not contain a value (either std::nullopt or std::null) 
		#define JSL_NULL_STRING "-null-"
	#endif

	WrapperConcept(OptionalLike, std::optional,is_optional )
	WrapperConcept(ChronoDuration, std::chrono::duration, is_time)
	WrapperConcept(UniquePtr, std::unique_ptr, is_unique_ptr)
	WrapperConcept(SharedPtr, std::shared_ptr, is_shared_ptr)

	template<typename T>
	concept SmartPointer = UniquePtr<T> || SharedPtr<T>;

	// template<typename T>
	// concept ChronoDuration = requires(T t)
	// {
	// 	typename T::rep;
	// 	typename T::period;
	// 	{ T{typename T::rep{}} };
	// };
}