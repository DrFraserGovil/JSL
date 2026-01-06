#pragma once
#include <vector>
#include <string_view>
#include <string>
#include <algorithm>
namespace JSL
{

    //! Removes leading or trailing whitespace from a string_view
    //! @param sv The original string_view
    //! @returns A modified string_view with no leading or trailing whitespace
    std::string_view inline trim(std::string_view sv)
    {
        // Find the first non-whitespace character
        size_t first = 0;
        while (first < sv.length() && std::isspace(static_cast<unsigned char>(sv[first])))
        {
            first++;
        }

        // Find the last non-whitespace character
        size_t last = sv.length();
        while (last > first && std::isspace(static_cast<unsigned char>(sv[last - 1])))
        {
            last--;
        }

        return sv.substr(first, last - first);
    }

    //! Removes leading or trailing whitespace from a string_view and trims any 'comments' from the string.
    //! @details Comments are signified by the 'commentIndicator', and all text after the indicator is removed.
    //! With indicator '#', the string "hello, my name is #Put your name here" would be trimmed to "hello my name is";
    //! @param sv The original string_view
    //! @param commentIndicator The string after which all text is to be removed.
    //! @returns A modified string_view with no leading or trailing whitespace
    std::string_view inline trim(std::string_view sv,const std::string & commentIndicator)
    {
        auto commentStart = sv.find(commentIndicator);
        if (commentStart != std::string::npos)
        {
            sv = sv.substr(0,commentStart);
        }
        return trim(sv);
    }

}
