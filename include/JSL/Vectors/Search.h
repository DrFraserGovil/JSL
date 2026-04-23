#pragma once
#include <numeric>      // std::iota
#include <algorithm>  
#include <math.h>
#include <vector>
namespace JSL
{
	//! A value used to indicate `not in array'
    static const size_t NotFound = std::numeric_limits<size_t>::max();

    //!Gets first id such that y[id] == x,  assuming that exact equality is well defined (see double override). \param x The value to be searched for \param y The vector to search through  \returns The index of the first element in the array which matches x. Returns JSL::NotFound (equivalent to string::npos) if no match found
	template<class T>
	inline size_t Find(T x, const std::vector<T> & y)
	{
		for (size_t j = 0; j < y.size(); ++j)
		{
			if (y[j] == x)
			{
				return j; 
			}
		}
		return NotFound;
	}
	

	//!Gets first id such that (y[id]- x) < tolerance. \param x The value to be searched for \param y The vector to search through \param tolerance The fractional difference permitted between two double values for them to be declared "approximately equal" \returns The index of the first element in the array which matches x. Returns JSL::NotFound (equivalent to string::npos) if no match found
	inline size_t Find(double x, const std::vector<double> & y, double tolerance)
	{
		for (size_t j = 0; j < y.size(); ++j)
		{
			if (std::abs(x - y[j]) <= tolerance)
			{
				return j; 
			}
		}
		return NotFound;
	}

   
}