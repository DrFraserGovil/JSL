#pragma once
#include <vector>

namespace JSL::Vector
{
    
    /// @brief Insert a copy of a vector at the end of a target vector
    /// @tparam T Any object which can be stored in a vector
    /// @param a The `host', which will be mutated to contain the new data
    /// @param b The `target', which will be inserted into the end of the host
    template <class T>
    inline void append(std::vector<T> & a, const std::vector<T> & b)
    {
        a.reserve(a.size()+b.size());
        a.insert(a.end(),b.begin(),b.end());
    };

	/// @brief Insert a copy of a vector at the beginning of a target vector
	/// @tparam T Any object which can be stored in a vector
	/// @param a The `host', which will be mutated to contain the new data
	/// @param b The `target', which will be inserted at the begining of the host
    template <class T>
    inline void prepend(std::vector<T> & a, const std::vector<T> & b)
    {
        a.reserve(a.size()+b.size());
        a.insert(a.begin(),b.begin(),b.end());
    };

    /// @brief Create a new vector which is the concatenation of the inputs
    /// @tparam T Any object which can be stored in a vector
    /// @param a The first vector in the sequence
    /// @param b The second vector in the sequence
    /// @return A single vector [contents(a), contents(b)]
    template<class T>
    std::vector<T> concat(const std::vector<T> & a, const std::vector<T> & b)
    {
        std::vector<T> out{a};
        append(out,b);
        return out;
    }
}