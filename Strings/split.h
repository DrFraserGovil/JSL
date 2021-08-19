#pragma once
#include <string>
#include <vector>
#include <sstream>

namespace JSL
{
	/*!
	 Splits the string based on the chosen delimiter. Repeated delimiters are ignored.
	 * \param s The input string to be split (unchanged)
	 * \param delimiter The delimiting character
	 * \returns A vector of non-empty strings. 
	*/
	inline std::vector<std::string> split(const std::string& s, char delimiter)
	{
		//dumb brute-force string splitter based on a delimiter
		std::vector<std::string> tokens;
		std::string token;
		std::istringstream tokenStream(s);
		while (getline(tokenStream, token, delimiter))
		{
			if( token.length() > 0) // empty rows not allowed
			{
				tokens.push_back(token);
			}
		}
		return tokens;
	};
}
