#include <JSL/Strings/Manipulate.h>
#include <JSL/internal/error.h>
#include <cctype>
namespace JSL
{
    /*
        Trimming functions
    */
    template<class T>
    T internalTrim(std::string_view sv)
    {
        // Find the first non-whitespace character
        size_t first = 0;
        while (first < sv.length() && std::isspace(static_cast<unsigned char>(sv[first])))
        {
            first++;
        }

        // Find the last non-whitespace character
        size_t last = sv.length();
        while (last > first && std::isspace(static_cast<unsigned char>(sv[last - 1])))
        {
            last--;
        }

        return static_cast<T>(sv.substr(first, last - first));
    }
    template<class T>
    T internalCommentStrip(std::string_view sv, std::string_view commentIndicator)
    {
        auto commentStart = sv.find(commentIndicator);
        if (commentStart != std::string_view::npos)
        {
            sv = sv.substr(0,commentStart);
        }
        return internalTrim<T>(sv);
    }


    std::string_view trim_view(std::string_view sv)
    {
        return internalTrim<std::string_view>(sv);
    }
    std::string trim(std::string_view sv)
    {
        return internalTrim<std::string>(sv);
    }

    
    std::string_view trim_view(std::string_view sv,std::string_view commentIndicator)
    {
        return internalCommentStrip<std::string_view>(sv,commentIndicator);
    }
    std::string trim(std::string_view sv,std::string_view commentIndicator)
    {
        return internalCommentStrip<std::string>(sv,commentIndicator);
    }


    /*
        Split functions   
    */

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