
#pragma once
#include <string>
#include <vector>
#include <type_traits> // For std::is_arithmetic_v, std::enable_if_t, etc.
#include <sstream>     // For std::stringstream for more controlled numeric to_string
#include <string_view> // For delimiter in vector case
 #include <concepts>
#include <ranges>

#include <charconv>
#include <memory>
#include <JSL/Concepts.h>
#include <JSL/internal/error.h>
namespace JSL::String
{
	/*! @brief Internal interface for Represent. 
		
	@details As with ParseTo we have to provide a wrapper struct to allow vector partial specialisation. 
		Default struct only applies to numeric types. Overloads handle the others
	*/


	template<typename T>
	struct RepresentStruct
	{
	};


	//macro to provide specialisations without boilerplate muddling it all up
	#define JSL_HAS_SPECIALISATION(type) \
		template<> struct RepresentStruct<type>{static std::string stringify (const type & value);}; 
	
	#define JSL_HAS_SPECIALISATION_NOCONST(type) \
		template<> struct RepresentStruct<type>{static std::string stringify ( type & value);};


	JSL_HAS_SPECIALISATION(bool);


	//a whole bunch of specialisations
	JSL_HAS_SPECIALISATION(int);
	JSL_HAS_SPECIALISATION(float);
	JSL_HAS_SPECIALISATION(double);
	JSL_HAS_SPECIALISATION(long);
	JSL_HAS_SPECIALISATION(long long);
	JSL_HAS_SPECIALISATION(long double);
	JSL_HAS_SPECIALISATION(unsigned long long);
	JSL_HAS_SPECIALISATION(unsigned long);
	JSL_HAS_SPECIALISATION(unsigned int);

	JSL_HAS_SPECIALISATION(char);
	JSL_HAS_SPECIALISATION(char*);
	JSL_HAS_SPECIALISATION_NOCONST(const char*);

	JSL_HAS_SPECIALISATION(std::string);
	JSL_HAS_SPECIALISATION(std::string_view);

	#undef JSL_HAS_SPECIALISATION


	template<Concept::NonStringRange T>
	struct RepresentStruct<T>
	{
		//! @brief Specialization for `std::vector<T_Inner>`
		//! @details Calls with default delimiter
		static std::string stringify(const T& container)
		{
			return stringify(container,", ");
		}
		
		//! @brief Specialization for `std::vector<T_Inner>`
		//! @details Calls with custom delimiter
		static std::string stringify(const T& container, std::string_view delimiter)
		{
			std::ostringstream result;
			result << "["; // A default endcap

			bool first = true;
			for (const auto& item : container)
			{
				if (!first)
				{
					result << delimiter;
				}
				
				// Extract the inner type and recurse
				using InnerType = std::remove_cvref_t<decltype(item)>;
				result << RepresentStruct<InnerType>::stringify(item);
				
				first = false;
			}

			result << "]";
			return result.str();
		}
	};

	/*! @brief Converts the object into a string. 
		@brief If the object is a vector, the default delimiter - "," - is used.
		@param obj An object (usually a numeric, boolean or vector) to be converted
		@returns A string representation
	*/
	template<typename T>
	std::string inline represent(T obj)
	{
		return RepresentStruct<T>::stringify(obj);
	};

	/*! @brief Converts a vector into a string
		@param obj An object (usually a numeric, boolean or vector) to be converted
		@param delimiter The character(s) which delimit the elements of the vector
		@returns A string representation
	*/
	template<typename T>
	std::string inline represent(T obj,std::string delimiter)
	{
		return RepresentStruct<T>::stringify(obj,delimiter);
	};

	
	/*! @brief Basic template for the makeFrom class of functions. 
	 * @tparam T An arbitrary type 
	 * @param obj An object to be stringified
	 * @throws template_errors if std::string not constructible from the object
	 * @return A string representing the object
	 */
	template<class T>
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
		constexpr size_t reserved = 32; // 15 characters + extra characters like +/-, exps etc. 
		const size_t needed = precision + 16;

		char stackbuffer[reserved];

		char * buf = nullptr;
		std::unique_ptr<char[], void(*)(void*)> heap_guard(nullptr,std::free);
		if (needed <= reserved)
		{
			buf = stackbuffer;
		}
		else
		{
			buf = (char*)malloc(needed*sizeof(char)); //shouldn't throw an error if the user makes a ludicrous demand, but will degrade performance to do this 
			
			heap_guard.reset(buf);
		}

		auto result = std::to_chars(buf, buf + needed, obj,fmt,precision);
		
		if (result.ec == std::errc()) //blank error = it worked
		{
			return std::string(buf,result.ptr-buf);
		}
		else
		{
			JSL::internal::FatalError("Bad numeric conversion",JSL_LOCATION) << "Could not convert " << obj << " to string";
			return "";
		} 
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
		constexpr size_t reserved = 24;
		char buf[reserved]{};
		std::to_chars_result result = std::to_chars(buf, buf + reserved, obj);

		if (result.ec != std::errc())
		{
			return makeFrom(obj,reserved);	
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

	/*! @brief Converts an iterable range into a string, recursively converting the contained objects
		@details The string has bracket endcaps ("[]") and a comma delimiter. For more fine grained control, see JSL::String::stitch
	 * @tparam T An object supporting range-based iteration (but not strings)
	 * @param obj The object to be converted
	 * @return A string representing the input object 
	 */
	template<JSL::Concept::NonStringRange T>
	std::string inline makeFrom(const T & obj)
	{
		std::ostringstream os;
		os << "[";
		bool first = true;
		for (auto & v : obj)
		{
			if (!first)
			{
				os << ", ";
			}
			first = false;
			os << v;
		}
		os << "]";
		return os.str();
	}
}