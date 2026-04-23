#pragma once

#include <string_view>
#include <cctype>   
namespace JSL
{

    //! Performs a case-insensitive equality check on two strings
    //! @returns True if a and b are (aside from cases) equal strings
    bool inline insensitiveEquals(const std::string_view a, const std::string_view b);
}