#pragma once
#include "Search.h"
namespace JSL::Vector
{


	/// @brief Finds all indices such that ``checker(vec[id]) == true``
	/// @tparam Predicate A function or lambda with signature <bool(r_member)>, where r_member is the type contained within R
	/// @tparam R an iterable container containing objects of type r_member
	/// @param vec The object to be search through
	/// @param checker A predicate function which acts on each element of ``vec`` and returns either true or false
	/// @return A vector of indices corresponding to all points where checker(vec[id]) == true. If no matches found, vector is empty
	template<Concept::SearchableRange R, class Predicate>
	inline std::vector<size_t> FindAllWhere(const R & vec, Predicate checker)
	{
		std::vector<size_t> out;
		size_t idx =0;

		for (const auto & val : vec)
		{
			if (checker(val))
			{
				out.push_back(idx);
			}
			++idx;
		}

		return out;
	}

	/// @brief Finds the indices of all matching values within the target
	/// @tparam T An object which can be stored in a container, and has an equality operator
	/// @tparam R An iterable container containing objects of type T
	/// @param vec The vector to be searched 
	/// @param target The values to be matched
	/// @return A vector of indices corresponding to all points where vec[id] == target. If no matches found, vector is empty
	template<typename T, Concept::SearchableRange R>
    requires std::convertible_to<T, std::ranges::range_value_t<R>>
	inline std::vector<size_t> FindAll(const R & vec, const T & target)
	{
		return FindAllWhere(vec,[&](auto & val){return val == target;}); 
	}	

	/// @brief Finds the indices of all matching values within the target up to the specified tolerance
	/// @param vec The vector to be searched 
	/// @param target The values to be matched
	/// @param tolereance Values considered equal if |a - b| < tolerance
	/// @return A vector of indices corresponding to all points where vec[id] ~= target. If no matches found, vector is empty
	inline std::vector<size_t> FindAll(std::vector<double> & vec, double target, double tolerance)
	{
		return FindAllWhere(vec,[&](auto & val){return std::abs(val-target) < tolerance;});
	}


}