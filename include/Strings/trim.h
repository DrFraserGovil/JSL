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
    std::string_view trim(std::string_view sv);

    //! Removes leading or trailing whitespace from a string_view and trims any 'comments' from the string.
    //! @details Comments are signified by the 'commentIndicator', and all text after the indicator is removed.
    //! With indicator '#', the string "hello, my name is #Put your name here" would be trimmed to "hello my name is";
    //! @param sv The original string_view
    //! @param commentIndicator The string after which all text is to be removed.
    //! @returns A modified string_view with no leading or trailing whitespace
    std::string_view trim(std::string_view sv,const std::string & commentIndicator);
}
