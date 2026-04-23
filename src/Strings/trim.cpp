#include "JSL/Strings/trim.h"

namespace JSL
{
    
    std::string_view trim(std::string_view sv)
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

        return sv.substr(first, last - first);
    }

    
    std::string_view trim(std::string_view sv,const std::string & commentIndicator)
    {
        //snip out from the comment indicator; then run a normal trim
        auto commentStart = sv.find(commentIndicator);
        if (commentStart != std::string::npos)
        {
            sv = sv.substr(0,commentStart);
        }
        return trim(sv);
    }

}
