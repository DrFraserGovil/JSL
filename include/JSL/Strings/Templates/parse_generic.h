#pragma once
#include <JSL/Strings/Templates/stringparsers.h>
#include <JSL/Strings/Manipulate.h>
#include <JSL/internal/error.h>
#include <JSL/Concepts/numerics.h>
#include <charconv>
namespace JSL::String
{
	/////////////////////
	// Generic Template
	/////////////////////
		
		/*! @brief The default template for parsing a string into a generic type
			@details Attempts a naive string-native construction. We don't require that such a construction exists as this is also our gateway to error messages
			@tparam T A type which can be converted into a string 
			@param sv A string to be parsed 
			@return An object of type T represented by the input string
		*/
		template<class T>
		T inline ParseTo(std::string_view sv)
		{
			sv=trim_view(sv);
			internal::RejectEmpty(sv,typeid(T).name());
			try 
			{
				return T{sv};
			}
			catch (...)
			{
			
				JSL::internal::FatalError("Bad conversion",JSL_LOCATION) << "Cannot convert " << sv << " to type " << typeid(T).name() << " using the default converter.";
				return ""; //dead code, suppresses compiler warnings
			}
		}
	
	/////////////////////
	// Numerics
	/////////////////////
	
		/*! @brief Parses strings into numeric types 
			@warning Uses std::to_chars, which may cause some issues with non-C++20 compliant compilers (Apple Clang, for example), for which this function is not implemented for doubles 
			@tparam T A type which can be converted into a string 
			@param sv A string to be parsed 
			@return An object of type T represented by the input string
		*/
		template<JSL::Concept::Numeric T>
		T inline ParseTo(std::string_view sv)
		{
			sv=trim_view(sv);
			internal::RejectEmpty(sv,typeid(T).name());
			T output{};
			auto result = std::from_chars(sv.data(),sv.data() + sv.size(), output);
			internal::CheckErrors(result,sv,typeid(T).name());

			return output;
		}
		
		/*! @brief Parses strings into std::chrono::duration objects 
			@warning This parses numeric values, and then passes them to the duration-constructor, and does not support primitives or any non-numeric values.  
			@tparam T A type which can be converted into a string 
			@param sv A string to be parsed 
			@return An object of type T represented by the input string
		*/
		template<JSL::Concept::ChronoDuration T>
		T inline ParseTo(std::string_view sv)
		{
			return T{ParseTo<typename T::rep>(sv)};
		}

	////////////////////
	// Specialisations
	////////////////////

		//The apple-clang override for doubles
		#if defined(__clang__) && defined(__APPLE__)
			template<>
			double ParseTo(std::string_view sv);
		#endif

		template<>
		char ParseTo(std::string_view sv);
		
		template<>
		bool ParseTo(std::string_view sv);

} // namespace JSL::String
