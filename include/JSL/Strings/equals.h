#pragma once

#include <string_view>
#include <cctype>   
namespace JSL
{
    namespace
    {
        bool inline insensitiveEqualsChar(char a, char b)
        {
            return std::tolower(static_cast<unsigned char>(a)) ==
                std::tolower(static_cast<unsigned char>(b));
        }
    }

    //! Performs a case-insensitive equality check on two strings
    //! @returns True if a and b are (aside from cases) equal strings
    bool inline insensitiveEquals(const std::string_view a, const std::string_view b)
    {
        return a.size() == b.size() &&
            std::equal(a.begin(), a.end(), b.begin(), insensitiveEqualsChar);
    }
}