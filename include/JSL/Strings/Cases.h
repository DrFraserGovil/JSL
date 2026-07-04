#pragma once

#include <string>
#include <string_view>

namespace JSL::String
{
    /// @brief Mutate an input string in-place to all-capitals
    /// @param input The string to be modified
    void toUpper(std::string & input);
    
    /// @brief Return a copy of an input string which is in all-capitals
    /// @param input the string to copy & modify
    /// @return A near-copy of ``input``, differing only by their cases
    std::string getUpper(std::string_view input);

    /// @brief Mutate an input string in-place to all-lowercase
    /// @param input The string to be modified
    void toLower(std::string & input);

    /// @brief Return a copy of an input string which is in all-lowercase
    /// @param input the string to copy & modify
    /// @return A near-copy of ``input``, differing only by their cases
    std::string getLower(std::string_view input);


    //! Performs a case-insensitive equality check on two strings
    //! @param a A view of a string to be compared
    //! @param b A view of a string to be compared
    //! @returns True if a and b are (aside from cases) equal strings
    bool iEquals(std::string_view a, std::string_view b);
} // namespace JSL
