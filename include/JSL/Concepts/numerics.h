#pragma once
#include <concepts>

namespace JSL::Concept
{
	//! @brief Any integral type (excluding bools, or chars and extensions thereof)
	template<typename T>
	concept Integer = std::integral<T>			&&
					!std::same_as<T, char> 		&&
					!std::same_as<T, wchar_t>	&&
					!std::same_as<T, char8_t>	&&
					!std::same_as<T, char16_t>	&&
					!std::same_as<T, char32_t>;

	

	//! @brief Any primitive numeric type (excluding bools, or chars and extensions thereof)
	template<typename T>
	concept Numeric = ( Integer<T> || std::floating_point<T>);  
}