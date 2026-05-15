#pragma once
#include <numeric>      // std::iota
#include <algorithm>  
#include <vector>
#include <ranges>
#include <concepts>
#include <functional>
namespace JSL::Vector
{
	/// @brief A signal that an element was not in the targeted array
	/// @details The assigned value is the maximum possible size_t (and so this functions like std::string::npos) 
	inline constexpr size_t NotFound = static_cast<size_t>(-1);

	/// @brief A container for storing search results, which can act simultaneously as the signal that a result was found, and as the index itself, with minimal friction 
	struct SearchResult
	{
		/// @brief True if the searched value exists within the input
		bool Found;
		/// @brief The index that the value can be found at (if it exists), else is NotFound
		size_t Index;
		/// @brief An explicit cast for converting the object into a boolean
		/// @details Allows for ``SearchResult found; if (found)....``
		explicit operator bool() const { return Found; } 
		/// @brief An implicit cast for converting the object into an index, allowing the object to be treated as a size_t object wherever it can.
		/// @warning This is only meaningful if the user first validates that Found is true!
		operator size_t() const { return Index; } //can do Vector[SearchResult] 
		
	};

	namespace internal
	{
		template<typename T>
		concept SearchableRange = std::ranges::input_range<T>;
	}

	/// @brief Gets the first id such that vec[id] == target
	/// @details An index is returned even if the container cannot be indexed (i.e. a std::list or std::set). The meaning of this is left to the user.
	/// @tparam T An object that can be stored in containers
	/// @tparam R A container of T-objects which can be iterated through
	/// @param vec The container to be searched through
	/// @param target The value to be searched for
	/// @return A SearchResult indicating if the element has been found, and the index it can be found at
	template<typename T, internal::SearchableRange R>
    requires std::convertible_to<T, std::ranges::range_value_t<R>>
	inline SearchResult find(const R & vec,const T & target )
	{
		auto begin = std::begin(vec);
		auto it = std::find(begin,std::end(vec),target);

		if (it != std::end(vec))
		{
			return {true,static_cast<size_t>(std::distance(begin,it))};
		}
		else
		{
			return {false,NotFound};
		}
	}
	
	/// @brief Finds the first id such that ``checker(vec[id]) == true``
	/// @tparam Predicate A function or lambda with signature <bool(r_member)>, where r_member is the type contained within R
	/// @tparam R an iterable container containing objects of type r_member
	/// @param vec The object to be search through
	/// @param checker A predicate function which acts on each element of ``vec`` and returns either true or false
	/// @return A SearchResult indicating where the first suitable element is, if one exists
	template<internal::SearchableRange R, class Predicate>
	inline SearchResult findWhere(const R & vec, Predicate checker )
	{
		auto begin = std::begin(vec);
		auto it = std::find_if(begin,std::end(vec),checker);
		
		if (it != std::end(vec))
		{
			return {true,static_cast<size_t>(std::distance(begin,it))};
		}
		else
		{
			return {false,NotFound};
		}
	}

	/// @brief An override for the general find algorithm for floating point values where exact equality is a dubious concept. Gets the first id such that ``std::abs(vec[id] - target) < tolerance``
	/// @param vec The container to be searched through
	/// @param target The value to be searched for
	/// @param tolerance The floating point error which counts as 'equality'
	/// @return A SearchResult indicating if the element has been found, and the index it can be found at
	SearchResult find(const std::vector<double> & vec, double target, double tolerance);



	/// @brief Detect if a container contains a specified value
	/// @tparam T An object which can be stored in a container, and has an equality operator
	/// @tparam R An iterable container containing objects of type T
	/// @param vec The vector to be searched 
	/// @param target The values to be matched
	/// @return True if the value is found, false otherwise
	template<typename T, internal::SearchableRange R>
    requires std::convertible_to<T, std::ranges::range_value_t<R>>
	inline bool contains(const R & vec, const T & target)
	{
		return static_cast<bool>(find(vec, target));
	}
   
}