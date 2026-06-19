#include <JSL/Strings/Split.h>
#include <JSL/internal/error.h>
namespace JSL::String
{
    //define a generic wrapper for string(_view)
    template<class T> 
    std::vector<T> internalsplitter(std::string_view input, std::string_view delimiter)
    {
        if (delimiter.size() == 0)
        {
            internal::FatalError("Split called with empty delimiter", JSL_LOCATION) ;
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

    //then the function definitions just specialise it
    std::vector<std::string_view> split_view(std::string_view input, std::string_view delimiter)
    {
        return internalsplitter<std::string_view>(input,delimiter);
    }

    std::vector<std::string> split(std::string_view input, std::string_view delimiter)
    {
        return internalsplitter<std::string>(input,delimiter);
    }
}