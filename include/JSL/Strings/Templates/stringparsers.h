#pragma once
#include <charconv>
#include <string_view>
#include <vector>
namespace JSL::String::internal
{
	void CheckErrors(std::from_chars_result & result,std::string_view sv,std::string_view typeName);
	void RejectEmpty(std::string_view sv,std::string_view typeName,bool isOptional = false);

	std::vector<std::string_view> tokenize(std::string_view sv, std::string_view delim,std::string_view typeName);
	std::vector<std::string_view> recursetokens(std::string_view sv);
	
}