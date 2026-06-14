#pragma once

#include <charconv>
#include <concepts>
#include <memory>
#include "JSL/Concepts/numerics.h"
#include "JSL/Concepts/optional.h"
#include "JSL/Concepts/pointers.h"
#include "JSL/Concepts/ranges.h"
#include "JSL/Concepts/strings.h"
#include "JSL/Concepts/tuple.h"
#include "SerialiserHelpers.h"
#include <cmath>

namespace JSL::String
{

	//////////////////////////
	// Generics
	//////////////////////////
		/*! @brief Basic template for the makeFrom class of functions. 
			@details Pipe the result into an output stream, and hope that if all else fails, a pipe has been implemented
		* @tparam T An arbitrary type 
		* @param obj An object to be stringified
		* @throws template_errors if std::string not constructible from the object
		* @return A string representing the object
		*/
		template<class T> std::string inline makeFrom(const T & obj)
		{
			std::ostringstream os;
			os << obj;
			return os.str();
		}

	//////////////////////////
	// Strings
	//////////////////////////

		/*! @brief A basic wrapper around a direct string-construction, simply to provide a unified interface 
		* @tparam T An arbitrary type 
		* @param obj An object to be stringified
		* @throws template_errors if std::string not constructible from the object
		* @return A string representing the object
		*/
		template<typename T> std::string inline makeFrom(const T & obj) requires JSL::Concept::StringType<T>
		{
			return std::string(obj);
		}
	
		/*! @brief Converts single chars into strings 
			@details Necessary because (for some reason) strings are not constructible from single chars 
		* @param ltr A character
		* @return A length-1 string containing the character
		*/
		template<> std::string inline makeFrom(const char & ltr)
		{
			return std::string(1,ltr);
		}

	//////////////////////////
	// Numerics
	//////////////////////////

		/*! @brief Converts floating point types into a string with a specified format 
		* @tparam T A floating point type
		* @param num The value to be stringified
		* @param precision The number of decimal places to retain 
		* @param fmt The format to be used (either fixed or scientific)
		* @return A string representing the input value
		* @throws std::runtime_error If the value cannot be converted to a string
		*/
		template<typename T>
		std::string inline makeFrom(const T & num, int precision, std::chars_format fmt = std::chars_format::general) requires std::floating_point<T>
		{
			const size_t needed = precision + 16; //16 is worst case scenario of required space for a  negative, hex with maximal radix
			constexpr size_t stackLimit = 30;
			std::to_chars_result result;
			if (needed < stackLimit)
			{
				char buffer[stackLimit];
				result = std::to_chars(buffer, buffer + needed, num,fmt,precision);
				if (result.ec == std::errc())
				{
					return std::string(buffer,result.ptr-buffer);
				}
			}
			else
			{
				std::vector<char> buffer(needed);
				result = std::to_chars(buffer.data(), buffer.data() + needed, num,fmt,precision);

				if (result.ec == std::errc())
				{
					return std::string(buffer.data(),result.ptr-buffer.data());
				}
			}

			throw std::logic_error("JSL conversion error: " + std::make_error_code(result.ec).message());
			return ""; //stops compiler warnings; not used
			
		}

		
		/*! @brief Converts integral types into a string with a specified format 
		* @details Since 'precision' is not defined for integers, this performs a direct cast to double, and hands it off to the floating point overload 
		* @tparam T An integral type (except bool and char)
		* @param val The value to be stringified
		* @param precision The precision to be used after casting the input to a scientific notation
		* @param fmt The formatting used after casting the input to a double (defaults to scientific)
		* @return A string representing the input value
		* @throws std::runtime_error If the value cannot be converted to a string
		*/
		template<typename T>
		std::string inline makeFrom(const T & val, int precision,std::chars_format fmt = std::chars_format::scientific) requires JSL::Concept::Integer<T>
		{
			//precision is ill-defined for integers; so we cast it to double and let the scientific notation switch happen
			return makeFrom<double>(static_cast<double>(val),precision,fmt);
		}

		/*! @brief Converts any numeric type into a string 
		* @details Uses the principle of 'least representation' to determine length of output string. For resolution control, see the overloads
		* @details Some floating points may end up larger than the reserved buffer can hold; this triggers a delegation to the more involved overload with a fixed precision.
		* @tparam T A numeric type (except bool and char)
		* @param num The value to be stringified
		* @return A string representing the input value
		* @throws std::runtime_error If the value cannot be converted to a string
		*/
		template<typename T> std::string inline makeFrom(const T & num) requires JSL::Concept::Numeric<T>
		{
			constexpr size_t reserved = std::max(
				std::numeric_limits<T>::max_digits10 + 9,
				std::numeric_limits<T>::digits10 + 3
			);
			char buf[reserved]{};
			std::to_chars_result result = std::to_chars(buf, buf + reserved, num);

			if (result.ec != std::errc())
			{
				throw std::logic_error("JSL conversion error: " + std::make_error_code(result.ec).message());
				return ""; //stops compiler warnings; not used
			}
			else
			{
				return std::string(buf, result.ptr - buf);
			}
		}
		
		/*! @brief Converts booleans into strings 
		* @param bln A boolean
		* @return Either "true" or "false"
		*/
		template<> std::string inline makeFrom(const bool & bln)
		{
			return bln ? "true" : "false";
		}

	/////////////////////////
	// Forward declarations
	// For nested types
	/////////////////////////
		template<typename T> std::string inline makeFrom(const T & vec) requires JSL::Concept::NonStringRange<T>;

		template<typename T> std::string inline makeFrom(const T & tpl) requires JSL::Concept::TupleLike<T>;
	
		template<typename T> std::string inline makeFrom(const T & opt) requires JSL::Concept::OptionalLike<T>;
	
		template<typename T> std::string inline makeFrom(const T & ptr) requires JSL::Concept::SmartPtr<T>;
	/////////////////////////
	// Containers 
	/////////////////////////

		/*! @brief Converts an iterable range into a string, recursively converting the contained objects
			@details The string has bracket endcaps ("[]") and a comma delimiter. For more fine grained control, see JSL::String::stitch
		* @tparam T An object supporting range-based iteration (but not strings)
		* @param vec The object to be converted (not necessarily a std::vector, despite the name)
		* @return A string representing the input array 
		*/
		template<typename T>
		std::string inline makeFrom(const T & vec) requires JSL::Concept::NonStringRange<T>
		{
			std::ostringstream os;
			os << "[";
			bool first = true;
			for (const auto & v : vec)
			{
				if (!first)
				{
					os << ", ";
				}
				first = false;
				os << makeFrom(v);
			}
			os << "]";
			return os.str();
		}
	///////////////////////
	// Tuples
	///////////////////////
		/*! @brief Converts a tuple into a string, recursively converting the contained objects	
			@details The string has bracket endcaps ("()") and a comma delimiter. 
			@tparam T Any std::tuple or std::pair object 
			@param tpl The value to be stringified
			@return A string representing the input value
			@throws std::runtime_error: If the value cannot be converted to a string
		*/
		template<typename T>
		std::string inline makeFrom(const T & tpl) requires JSL::Concept::TupleLike<T>
		{
			std::ostringstream os;
			os << "(";
			std::apply([&os, first = true](const auto&... args) mutable
			{
				((os << (first ? first = false, "" : ", ") << makeFrom(args)), ...);
			}, tpl);
			os << ")";
			return os.str();
		}
	/////////////////////
	// Nullable Wrappers
	/////////////////////
		/*! @brief Converts an optional-value into a string, returning either the value (if it exists), or the NULL STRING.	
			@tparam T Any std::optional object 
			@param opt The value to be stringified
			@return A string representing the input value
			@throws std::runtime_error: If the internal type is not supported, or cannot be converted to a string
		*/
		template<typename  T>
		std::string inline makeFrom(const T & opt) requires JSL::Concept::OptionalLike<T>
		{
			if (!opt){return JSL_NULL_STRING;}
			else{return makeFrom(opt.value());}
		}
		
		/*! @brief Converts the value held by a smart pointer a into a string, returning either the value (if it exists), or the NULL STRING.	
			@details This stringifies the object pointed to by the pointer, not the pointer itself
			@tparam T Any std::unique_ptr or std::shared_ptr objectj 
			@param ptr The pointer associated with the object to be stringified
			@return A string representing the pointed-to value
			@throws std::runtime_error: If the internal type is not supported, or cannot be converted to a string
		*/
		template<typename  T>
		std::string inline makeFrom(const T & ptr) requires JSL::Concept::SmartPtr<T>
		{
			if (!ptr){return JSL_NULL_STRING;}
			else{return makeFrom(*ptr);}
		}
}