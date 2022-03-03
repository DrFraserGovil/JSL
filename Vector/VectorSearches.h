#pragma once
#include <numeric>      // std::iota
#include <algorithm>  


namespace JSL
{
	//gets first id such that y[id] == x. If no such id exists, returns negative value
	template<class T>
	inline int FindXInY(T x, const std::vector<T> & y)
	{
		for (int j = 0; j < y.size(); ++j)
		{
			if (abs((y[j] - x)/x) < 1e-6)
			{
				return j; 
			}
		}
		return -1;
	};
	
	template <typename T>
	inline  std::vector<size_t> SortIndices(const std::vector<T> &v) {
	
	  // initialize original index locations
	  std::vector<size_t> idx(v.size());
	  std::iota(idx.begin(), idx.end(), 0);
	
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
