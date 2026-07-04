
#pragma once
#include <vector>
#include <string_view>
#include <string>
namespace JSL::String
{
    
    /*! @brief Splits a string into a vector, with each element indicated by the delimiter string
        @warning The output is a vector of string_views - references to the original string. This has the limitation that the output is only meaningful so long as the original string survives. Copies out of the string_view do persist
        @param input A (view) of a string to be split
        @param delimiter The string which indicates a 'break'. Delimiters do not appear in the split output
        @returns A vector of windows into the original string, indicating which elements have been grouped together.
    */
    std::vector<std::string_view> split_view(std::string_view input, std::string_view delimiter);

    /*! @brief As with split_view, with the exception that the values are cast to strings, and hence persist after the original input has passed out of scope
        @param input A (view) of a string to be split
        @param delimiter The string which indicates a 'break'. Delimiters do not appear in the split output
        @returns A vector of strings 
    */
    std::vector<std::string> split(std::string_view input,std::string_view delimiter);
} 