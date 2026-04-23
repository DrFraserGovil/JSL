#include <vector>
#include <string_view>
#include <stdexcept>
#include "../utils/jsl_error.h"

namespace JSL
{
 


    template<class T> 
    std::vector<T> internalsplitter(std::string_view input, std::string_view delimiter)
    {
        if (delimiter.size() == 0)
        {
            internal::FatalError("Split called with empty delimiter");
        }
        std::vector<T> tokens;
        size_t start = 0;
        size_t end = input.find(delimiter);
        size_t delim_len = delimiter.length();

        while (end != std::string_view::npos)
        {
            tokens.emplace_back(input.substr(start, end - start));
            start = end + delim_len;
            end = input.find(delimiter, start);
        }
        tokens.emplace_back(input.substr(start)); // Add the last token
        return tokens;
    }

     std::vector<std::string_view> split_view(std::string_view input, std::string_view delimiter)
     {
        return internalsplitter<std::string_view>(input,delimiter);
     }

    std::vector<std::string> split(std::string_view input, std::string_view delimiter)
    {
        return internalsplitter<std::string>(input,delimiter);
    }
}