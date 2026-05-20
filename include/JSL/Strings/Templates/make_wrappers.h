#pragma once
#include <JSL/Strings/MakeFrom.h>
#include <JSL/Concepts/pointers.h>
namespace JSL::String
{
	/*! @brief Converts an optional-value into a string, returning either the value (if it exists), or the NULL STRING.	
		@tparam T Any std::optional object 
		@param obj The value to be stringified
		@return A string representing the input value
		@throws std::runtime_error: If the internal type is not supported, or cannot be converted to a string
	*/
	template<JSL::Concept::OptionalLike T>
	std::string inline makeFrom(const T & obj)
	{
		if (!obj){return JSL_NULL_STRING;}
		else{return makeFrom(obj.value());}
	}
	
	/*! @brief Converts the value held by a smart pointer a into a string, returning either the value (if it exists), or the NULL STRING.	
		@details This stringifies the object pointed to by the pointer, not the pointer itself
		@tparam T Any std::unique_ptr or std::shared_ptr objectj 
		@param obj The pointer associated with the object to be stringified
		@return A string representing the pointed-to value
		@throws std::runtime_error: If the internal type is not supported, or cannot be converted to a string
	*/
	template<JSL::Concept::SmartPtr T>
	std::string inline makeFrom(const T & obj)
	{
		if (!obj){return JSL_NULL_STRING;}
		else{return makeFrom(*obj);}
	}
}