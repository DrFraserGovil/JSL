#pragma once
#include <vector>
#include <string_view>
#include <stdexcept>


namespace JSL
{
    /*! @brief Splits a string into a vector, with each element indicated by the delimiter string
        @warning The output is a vector of string_views - references to the original string. This has the limitation that the output is only meaningful so long as the original string survives. Copies out of the string_view do persist
        @param string A (view) of a string to be split
        @param delimiter The string which indicates a 'break'. Delimiters do not appear in the split output
        @returns A vector of windows into the original string, indicating which elements have been grouped together.
    */
    std::vector<std::string_view> split(std::string_view string, std::string_view delimiter)
    {
        if (delimiter.size() == 0)
        {
            throw std::runtime_error("Split called with empty delimiter");
        }
        std::vector<std::string_view> tokens;
        size_t start = 0;
        size_t end = string.find(delimiter);
        size_t delim_len = delimiter.length();

        while (end != std::string_view::npos) {
            tokens.push_back(string.substr(start, end - start));
            start = end + delim_len;
            end = string.find(delimiter, start);
        }
        tokens.push_back(string.substr(start)); // Add the last token
        return tokens;
    }
}