#pragma once
#include <concepts>
#include <chrono>
namespace JSL::Concept
{
	//! @brief Any integral type (excluding bools, or chars and extensions thereof)
	template<typename T>
	concept Integer = std::integral<T>			&&
					!std::same_as<T, bool>		&&
					!std::same_as<T, char> 		&&
					!std::same_as<T, wchar_t>	&&
					!std::same_as<T, char8_t>	&&
					!std::same_as<T, char16_t>	&&
					!std::same_as<T, char32_t>;

	

	//! @brief Any primitive numeric type (excluding bools, or chars and extensions thereof)
	template<typename T>
	concept Numeric = ( Integer<T> || std::floating_point<T>); 
	
	

	
	namespace internal
	{
		template<typename T> struct is_time : std::false_type {};
		template<typename...Args> struct is_time<std::chrono::duration<Args...>> : std::true_type  {};
	}

	template<typename T> concept ChronoDuration = internal::is_time<T>::value;
}