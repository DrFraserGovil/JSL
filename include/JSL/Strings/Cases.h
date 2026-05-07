#pragma once

#include <string>
#include <string_view>

namespace JSL::String
{

    //! Converts a string in-place to all UPPERCASE
    void toUpper(std::string & input);
    std::string getUpper(std::string_view input);
    void toLower(std::string & input);
    std::string getLower(std::string_view input);


    //! Performs a case-insensitive equality check on two strings
    //! @param a A view of a string to be compared
    //! @param b A view of a string to be compared
    //! @returns True if a and b are (aside from cases) equal strings
    bool iEquals(std::string_view a, std::string_view b);
} // namespace JSL
