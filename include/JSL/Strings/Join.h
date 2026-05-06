#pragma once
#include <string>
#include <vector>
#include <JSL/Strings/Represent.h>
#include <sstream>
namespace JSL::String
{
    template<class T>
    std::string join(const std::vector<T> & vec, size_t begin, size_t end, std::string_view delim)
    {
        if (begin >= end || begin >= vec.size())
        {
            return "";
        }
        std::ostringstream os;
        os << represent(vec[begin]);
        for (size_t i = begin + 1; i < end && i < vec.size(); ++i)
        {
            os << delim << represent(vec[i]);
        }
        return os.str();
    }

    template<class T>
    std::string join(const std::vector<T> & vec, std::string_view delim)
    {
        return join(vec, 0, vec.size(), delim);
    }
     
}