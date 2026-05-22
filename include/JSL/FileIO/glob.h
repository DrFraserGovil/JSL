#pragma once
#include <regex>
#include <JSL/Concepts/strings.h>
#include <JSL/Display/Log.h>
namespace JSL::IO
{
	template<JSL::Concept::StringType T>
	std::string inline makeGlob(T & input)
	{
		std::string result ="^";
		for (auto c : input)
		{
			if (c== '\0') // if T is a char * we can end up with null terminators ending up in our sequences
			{
				break;
			}

			switch(c)
			{
				case '*': result += ".*"; break;
				case '?': result += ".";  break;
				// Characters that need escaping in regex
				case '.': case '+': case '^': case '$':
				case '|': case '(': case ')': case '[':
				case ']': case '{': case '}': case '\\':
					result += '\\';
					result += c;
					break;
				default: result += c;
			}			
		}
		LOG(INFO) << result + "$";
		return result + "$";
	}

	template<JSL::Concept::StringType T>
	std::regex inline globToRegex(T & input)
	{
		return std::regex(makeGlob(input));
	}
}