#pragma once
#include <vector>

namespace JSL
{
    template <class T>
    inline void append(std::vector<T> & a, const std::vector<T> & b)
    {
        a.reserve(a.size()+b.size());
        a.insert(a.end(),b.begin(),b.end());
    };

    template <class T>
    inline void prepend(std::vector<T> & a, const std::vector<T> & b)
    {
        a.reserve(a.size()+b.size());
        a.insert(a.begin(),b.begin(),b.end());
    };

    template<class T>
    std::vector<T> concat(const std::vector<T> & a, const std::vector<T> & b)
    {
        std::vector<T> out{a};
        append(out,b);
        return out;
    }
}