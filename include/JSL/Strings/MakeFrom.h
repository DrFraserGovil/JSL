
#pragma once

#include <charconv>
#include <memory>
#include <JSL/Concepts/strings.h>
#include <JSL/Concepts/numerics.h>
#include <JSL/internal/error.h>
#include <cmath>



namespace JSL::String
{


	/*! @brief Basic template for the makeFrom class of functions. 
		@details Pipe the result into an output stream, and hope that if all else fails, a pipe has been implemented
	 * @tparam T An arbitrary type 
	 * @param obj An object to be stringified
	 * @throws template_errors if std::string not constructible from the object
	 * @return A string representing the object
	 */
	template<class T>
	std::string inline makeFrom(const T & obj)
	{
		std::ostringstream os;
		os << obj;
		return os.str();
	}

	/*! @brief A basic wrapper around a direct string-construction, simply to provide a unified interface 
	 * @tparam T An arbitrary type 
	 * @param obj An object to be stringified
	 * @throws template_errors if std::string not constructible from the object
	 * @return A string representing the object
	 */
	template<JSL::Concept::StringType T>
	std::string inline makeFrom(const T & obj)
	{
		return std::string(obj);
	}

	/*! @brief Converts floating point types into a string with a specified format 
	 * @tparam T A floating point type
	 * @param obj The value to be stringified
	 * @param precision The number of decimal places to retain 
	 * @param fmt The format to be used (either fixed or scientific)
	 * @return A string representing the input value
	 * @throws std::runtime_error If the value cannot be converted to a string
	 */
	template<std::floating_point T>
	std::string inline makeFrom(const T & obj, int precision, std::chars_format fmt = std::chars_format::general)
	{
		const size_t needed = precision + 16; //16 is worst case scenario of required space for a  negative, hex with maximal radix
		constexpr size_t stackLimit = 30;
		if (needed < stackLimit)
		{
			char buffer[stackLimit];
			auto result = std::to_chars(buffer, buffer + needed, obj,fmt,precision);
			if (result.ec == std::errc())
			{
				return std::string(buffer,result.ptr-buffer);
			}
		}
		else
		{
			std::vector<char> buffer(needed);
			auto result = std::to_chars(buffer.data(), buffer.data() + needed, obj,fmt,precision);

			if (result.ec == std::errc())
			{
				return std::string(buffer.data(),result.ptr-buffer.data());
			}
		}
		JSL::internal::FatalError("Bad numeric conversion",JSL_LOCATION) << "Could not convert " << obj << " to string";
		return ""; //stops compiler warnings; not used
		
	}

	
	/*! @brief Converts integral types into a string with a specified format 
	 * @details Since 'precision' is not defined for integers, this performs a direct cast to double, and hands it off to the floating point overload 
	 * @tparam T An integral type (except bool and char)
	 * @param obj The value to be stringified
	 * @return A string representing the input value
	 * @throws std::runtime_error If the value cannot be converted to a string
	 */
	template<JSL::Concept::Integer T>
	std::string inline makeFrom(const T & obj, int precision,std::chars_format fmt = std::chars_format::general)
	{
		//precision is ill-defined for integers; so we cast it to double and let the scientific notation switch happen
		return makeFrom<double>(static_cast<double>(obj),precision,fmt);
	}

	/*! @brief Converts any numeric type into a string 
	 * @details Uses the principle of 'least representation' to determine length of output string. For resolution control, see the overloads
	 * @details Some floating points may end up larger than the reserved buffer can hold; this triggers a delegation to the more involved overload with a fixed precision.
	 * @tparam T A numeric type (except bool and char)
	 * @param obj The value to be stringified
	 * @return A string representing the input value
	 * @throws std::runtime_error If the value cannot be converted to a string
	 */
	template<JSL::Concept::Numeric T>
	std::string inline makeFrom(const T & obj)
	{
		constexpr size_t reserved = std::max(
			std::numeric_limits<T>::max_digits10 + 9,
			std::numeric_limits<T>::digits10 + 3
		);
		char buf[reserved]{};
		std::to_chars_result result = std::to_chars(buf, buf + reserved, obj);

		if (result.ec != std::errc())
		{
			JSL::internal::FatalError("Bad numeric conversion",JSL_LOCATION) << "Could not convert " << obj << " to string";
			return ""; //stops compiler warnings; not used
		}
		else
		{
			return std::string(buf, result.ptr - buf);
		}
	}
	
	/*! @brief Converts booleans into strings 
	 * @param obj A boolean
	 * @return Either "true" or "false"
	 */
	template<>
	std::string inline makeFrom(const bool & obj)
	{
		return obj ? "true" : "false";
	}

	/*! @brief Converts single chars into strings 
		@details Necessary because (for some reason) strings are not constructible from single chars 
	 * @param obj A character
	 * @return A length-1 string containing the character
	 */
	template<>
	std::string inline makeFrom(const char & obj)
	{
		return std::string(1,obj);
	}


}
#include <JSL/Strings/Templates/make_containers.h>
#include <JSL/Strings/Templates/make_tuple.h>
#include <JSL/Strings/Templates/make_wrappers.h>