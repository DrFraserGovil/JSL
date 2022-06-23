#pragma once
#include <numeric>      // std::iota
#include <algorithm>  
#include <math.h>

namespace JSL
{
	//!Gets first id such that y[id] == x,  assuming that exact equality is well defined (see double override). If no such id exists, returns negative value \param x The value to be searched for \param y The vector to search through  \returns The index of the first element in the array which matches x. Value is negative if no match found
	template<class T>
	inline int FindXInY(T x, const std::vector<T> & y)
	{
		for (int j = 0; j < y.size(); ++j)
		{
			if (y[j] == x)
			{
				return j; 
			}
		}
		return -1;
	};
	
	//!Gets first id such that (y[id]- x)/x < tolerance. If no such id exists, returns negative value. \param x The value to be searched for \param y The vector to search through \param tolerance The fractional difference permitted between two double values for them to be declared "approximately equal" \returns The index of the first element in the array which matches x. Value is negative if no match found
	inline int FindXInY(double x, const std::vector<double> & y, double tolerance)
	{
		for (int j = 0; j < y.size(); ++j)
		{
			double diff = abs(x - y[i])/(1e-99 +x);
			if (diff <= tolerance)
			{
				return j; 
			}
		}
		return -1;
	}

	/*!
		Returns the sorted index array associated with a vector - not the sorted array itself \param v A vector of objects where the less than operator is defined \returns a sorted index-vector y such that v[y[0]] is the smallest value in the array, v[y[1]] is the next, and so on. 
	*/
	template <typename T>
	inline  std::vector<size_t> SortIndices(const std::vector<T> &v) {
	
	  // initialize original index locations
	  std::vector<size_t> idx(v.size());
	  std::iota(idx.begin(), idx.end(), 0); //iota is essentially C++'s range function
	

		//I think I grabbed this from stack overflow.....
	  // sort indexes based on comparing values in v
	  // using std::stable_sort instead of std::sort
	  // to avoid unnecessary index re-orderings
	  // when v contains elements of equal values 
	  stable_sort(idx.begin(), idx.end(),
	       [&v](size_t i1, size_t i2) {return v[i1] < v[i2];});
	
	  return idx;
	};
	
	
	
	//!Similar to FindXInY except where you do not expect an exact match. Searches through an (assumed sorted) vector and locates the first value greater than or equal to the target value, else returns the index of the final value in the array
	inline int UpperBoundLocator(double val, const std::vector<double> & valArray)
	{
		int id = 0;
		bool stillSearching = true;
		while (stillSearching)
		{
			if (valArray[id] >= val)
			{
				stillSearching = false;
			}
			else
			{
				++id;
				if (id>= valArray.size())
				{
					stillSearching = false;
					id = valArray.size() - 1;
				}
			}
		}
		return id;
	};
}
