#include <JSL/Strings/Trim.h>
#include <JSL/internal/error.h>
namespace JSL::String
{
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


}