#pragma once
#include <string_view>
#include <string>
namespace JSL::String
{

    //! Removes leading or trailing whitespace from a string_view
    //! @warning The output is a string_view - reference to the original string. This has the limitation that the output is only meaningful so long as the original string survives. Copies out of the string_view do persist, or use trim.
    //! @param sv The original string_view
    //! @returns A modified string_view with no leading or trailing whitespace
    std::string_view trim_view(std::string_view sv);
    
    //! Removes leading or trailing whitespace from a string_view
    //! @param sv The original string_view
    //! @returns A modified string_view with no leading or trailing whitespace
    std::string trim(std::string_view sv);

    //! Removes leading or trailing whitespace from a string_view and trims any 'comments' from the string.
     //! @warning The output is a string_view - a reference to the original string. This has the limitation that the output is only meaningful so long as the original string survives. Copies out of the string_view do persist, or use trim.
    //! @details Comments are signified by the 'commentIndicator', and all text after the indicator is removed.
    //! With indicator '#', the string "hello, my name is #Put your name here" would be trimmed to "hello, my name is";
    //! @param sv The original string_view
    //! @param commentIndicator The string after which all text is to be removed.
    //! @returns A modified string_view with no leading or trailing whitespace and no comments
    std::string_view trim_view(std::string_view sv,std::string_view commentIndicator);

    //! Removes leading or trailing whitespace from a string_view and trims any 'comments' from the string.
    //! @details Comments are signified by the 'commentIndicator', and all text after the indicator is removed.
    //! With indicator '#', the string "hello, my name is #Put your name here" would be trimmed to "hello my name is";
    //! @param sv The original string_view
    //! @param commentIndicator The string after which all text is to be removed.
    //! @returns A modified string with no leading or trailing whitespace and no comments
    std::string trim(std::string_view sv,std::string_view commentIndicator);

} // namespace JSL
