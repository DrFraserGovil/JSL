#pragma once
#include <regex>
#include <JSL/Concepts/strings.h>
namespace JSL::IO
{
	/*! @brief Converts a glob-string into a regex-string
		@tparam T A string-like type (string, string_view, const char*)
		@details '*' and '?' are converted into their glob equivalent, and other regex-sensitive characters are escaped.
		@param input A glob-string to be converted
		@param returns A string representing the glob in standard regex
	*/	
	template<JSL::Concept::StringType T>
	std::string inline makeGlob(T & input)
	{
		std::string result ="^";
		bool escaped = false;
		bool firstopen = false;
		bool open = false;

		for (auto c : input)
		{
			if (c== '\0') // if T is a char * we can end up with null terminators ending up in our sequences
			{
				break;
			}
			if (open && !escaped)
			{
				
				if (firstopen)
				{
					firstopen = false;
				
					if (c == '!')
					{
						result += '^';
						continue;
					}
					if (c == ']')
					{
						result += '\\';
						result += c;
						continue;
					}
				}

				switch(c)
				{
					case '*': case '?': result += c; continue;
					case ']': open = false; break;
					default: break;
				}
				
			}
			if (escaped)
			{
				result += c;
				escaped = false;
			}
			else
			{
				switch(c)
				{
					case '*': result += ".*"; break;
					case '?': result += ".";  break;
					case '\\': escaped = true; break;
					case '[': open = true; firstopen = true; result += c; break;
					// Characters that need escaping in regex
					case '.': case '+': case '^': case '$':
					case '|': case '(': case ')': case '{': case '}':
						result += '\\';
						result += c;
						break;
					default: result += c;
				}			
			}
		}
		return result + "$";
	}

	/*! @brief Converts a glob-string into a regex object
		@tparam A string-like type (string, string_view, const char*)
		@details '*' and '?' are converted into their glob equivalent, and other regex-sensitive characters are escaped.
		@param input A glob-string to be converted
		@param returns A compiled regex object representing the globstring
	*/	
	template<JSL::Concept::StringType T>
	std::regex inline globToRegex(T & input)
	{
		return std::regex(makeGlob(input));
	}
}