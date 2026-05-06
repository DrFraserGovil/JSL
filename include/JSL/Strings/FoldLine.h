#pragma once

#include <string_view>
#include <vector>

namespace JSL::String
{
    
    /*!
        @brief Returns the true width of a string, taking into account tab characters and escape sequences
        @warning This function makes the assumption that the tab size used by the output renderer, and all control sequences are of the form \x1b...m. This is a common format for terminal control sequences, but may not be universally true. Use with caution.
    */
    size_t trueSize(std::string_view str,size_t tabSize = 4);

    /*!
        @brief Folds a string to a specified width, taking into account tab characters and escape sequences. 
        @details Lines shorter than the width are padded with spaces to ensure they are meet this minimum width. Lines will only be folded at whitespace characters, so if a single word is longer than the width, it will be placed on a line by itself and exceed the width limit. The returned vector contains views into the original string, so no copying of the string data is performed.
        @warning This function makes the assumption that the tab size used by the output renderer, and all control sequences are of the form \x1b...m. This is a common format for terminal control sequences, but may not be universally true. Use with caution.
    */
    std::vector<std::string> foldToWidth(std::string_view str, size_t width);


    /*
        Prints the lines side-by-side, folding them all to fit to the corresponding width
    */
    void columnPrint(std::vector<std::string_view> input, std::vector<size_t> widths, std::string_view delimiter);
    
    /*
        As with columnPrint, but all the columns are the same width
    */
    void columnPrint(std::vector<std::string_view> input, size_t width, std::string_view delimiter);

}