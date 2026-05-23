#pragma once
#include <JSL/Concepts.h>
#include "nullstring.h"
#include <charconv>
namespace JSL::String
{
	/////Helpers
	namespace internal
	{
		/*!	@brief Interprets the output from the inbuilt parsing call
			@param result The output from a call to from_chars
			@param sv The string being parsed
			@param typeName The (possibly mangled) typeid name of the target type
		*/	
		void checkErrors(std::from_chars_result & result,std::string_view sv,std::string_view typeName);


		/*! @brief Trims the string, then checks if the input string is empty, or a reserved sequence, and throw an error if true.  
		 * @param sv The string to be parsed - is mutated by a whitespace trimming
		 * @param typeName THe name of the target type
		 * @param rejectEmpty If true, strings that are empty after trimming throw an error
		 * @param isOptional True if the target type is std::optional<T>, otherwise false
		 * @throws std::logic_error if the string is empty and rejectEmpty is true
		 * @throws std::logic_error if sv == "__bool_tag__" but the type is not bool 
		 * @throws std::logic_error sv is the Null String, and isOptional is false 
		 */
		void processInput(std::string_view & sv, std::string_view type, bool rejectEmpty, bool isOptional=false);

		/*! @brief Splits the innermost type of a container-parse into 'tokens' to be parsed 
			@param sv The string to be tokenised
			@param delimiter The string used to determine the end of a token and the beginning of the next
			@param typeName The (possibly mangled) name of the parse target type. Used for error messages. 
		*/
		std::vector<std::string_view> tokenize(std::string_view sv, std::string_view delim,std::string_view typeName);
		/*!
		 * @brief Recursively break nested container types into lower-levels for sequential parsing
		 * @param sv The string to be meta-tokenised
		 * @return A vector of tokens suitable for further tokenisation by the inner types.
		 */
		std::vector<std::string_view> recursetokens(std::string_view sv);
	}
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
			internal::processInput(sv,typeid(T).name(),true);
			try 
			{
				return T{sv};
			}
			catch (...)
			{
				throw std::logic_error("Bad conversion, cannot convert string \"" + std::string(sv) + "\"");
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
			internal::processInput(sv,typeid(T).name(),true);
			T output{};
			auto result = std::from_chars(sv.data(),sv.data() + sv.size(), output);
			internal::checkErrors(result,sv,typeid(T).name());

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

	/////////////////////////
	// forward declarations
	/////////////////////////

		template<JSL::Concept::NonStringRange T>
		T ParseTo(std::string_view sv);

		template<JSL::Concept::TupleLike T>
		T ParseTo(std::string_view sv);
		template<JSL::Concept::TupleLike T>
		T ParseTo(std::string_view sv,std::string_view delim);
		
		template<JSL::Concept::OptionalLike T>
		T ParseTo(std::string_view sv);
		template<JSL::Concept::UniquePtr T>
		T ParseTo(std::string_view sv);
		template<JSL::Concept::SharedPtr T>
		T ParseTo(std::string_view sv);
	/////////////////////
	// Containers
	/////////////////////

		template<JSL::Concept::NonStringRange T>
		T inline ParseTo(std::string_view sv,std::string_view delimiter)
		{
			using InnerT = std::ranges::range_value_t<T>;
			std::vector<std::string_view> tokens;
			
			
			T out{};
			if constexpr (JSL::Concept::NonStringRange<InnerT> || JSL::Concept::TupleLike<InnerT>)
			{
				tokens = (internal::recursetokens(sv));
				if (tokens.empty() && !sv.empty())
				{
					throw std::logic_error("Bad container parse: The string \""  + std::string(sv) + "\" is non-empty, but no matching container boundaries can be found");
				}
			}
			else
			{ 
				tokens = (internal::tokenize(sv,delimiter,typeid(T).name()));	
			}
			if (tokens.empty())
			{
				return out;
			}		


			std::transform(tokens.begin(),tokens.end(),std::inserter(out,out.end()),
				[&](const auto & token)
				{
					if constexpr (JSL::Concept::NonStringRange<InnerT> || JSL::Concept::TupleLike<InnerT>)
					{
						return JSL::String::ParseTo<InnerT>(token, delimiter);
					}
					else
					{
						return JSL::String::ParseTo<InnerT>(token);
					}
				}	
			);
			return out;
		}
		
		template<JSL::Concept::NonStringRange T>
		T inline ParseTo(std::string_view sv)
		{
			return ParseTo<T>(sv,",");
		}
	

		namespace internal
		{
			template<JSL::Concept::TupleLike T, std::size_t... Is>
			T ParseToTupleImpl(const std::vector<std::string_view>& tokens, std::index_sequence<Is...>)
			{
				return T{ JSL::String::ParseTo<std::tuple_element_t<Is, T>>(tokens[Is])... };
			}
		}

		template<JSL::Concept::TupleLike T>
		T ParseTo(std::string_view sv,std::string_view delim)
		{
			auto tokens = internal::tokenize(sv,delim,typeid(T).name());

			if (tokens.size() != std::tuple_size_v<T>)
			{
				throw std::logic_error("Tuple parse error: number of tokens inconsistent with tuple dimensions");
			}

			return internal::ParseToTupleImpl<T>(tokens,std::make_index_sequence<std::tuple_size_v<T>>{});
		}
		template<JSL::Concept::TupleLike T>
		T ParseTo(std::string_view sv)
		{
			return ParseTo<T>(sv,",");
		}	
		
	////////////////////////
	// Optional
	///////////////////////
	
		template<JSL::Concept::OptionalLike T>
		T ParseTo(std::string_view sv)
		{
			internal::processInput(sv,typeid(T).name(),false);
			//no reject empty as we allow an empty signal to also be a failure

			if (sv.empty() || sv == JSL_NULL_STRING)
			{
				return std::nullopt;
			}

			return ParseTo<typename T::value_type>(sv);
		}
	
	///////////////////
	// Smart Pointers
	///////////////////

		template<JSL::Concept::UniquePtr T>
		T ParseTo(std::string_view sv)
		{
			using type = typename T::element_type;
			if (sv.empty() || sv == JSL_NULL_STRING)
			{
				return std::unique_ptr<type>(nullptr);
			}
			return std::make_unique<type>(std::move(ParseTo<type>(sv)));
		}
		template<JSL::Concept::SharedPtr T>
		T ParseTo(std::string_view sv)
		{
			using type = typename T::element_type;
			if (sv.empty() || sv == JSL_NULL_STRING)
			{
				return std::shared_ptr<type>(nullptr);
			}
			return std::make_shared<type>(std::move(ParseTo<type>(sv)));
		}
} // namespace JSL::String


//Some of these files are tightly coupled and recursive; they are broken up into blocks

// #include <JSL/Strings/Templates/parse_generic.h>
// #include <JSL/Strings/Templates/parse_containers.h>
// #include <JSL/Strings/Templates/parse_tuples.h>
// #include <JSL/Strings/Templates/parse_optional.h>
// #include <JSL/Strings/Templates/parse_ptr.h>



