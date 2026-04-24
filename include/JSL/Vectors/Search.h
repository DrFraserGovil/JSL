#pragma once
#include <numeric>      // std::iota
#include <algorithm>  
#include <vector>
#include <ranges>
#include <concepts>
namespace JSL
{
	inline constexpr size_t NotFound = static_cast<size_t>(-1);
	struct SearchResult
	{
		bool Found;
		size_t Index;
		explicit operator bool() const { return Found; } //can do if(SearchResult) and it works
	};

	namespace internal
	{
		template<typename T>
		concept SearchableRange = std::ranges::input_range<T>;
	}

 
	
	//!Gets first id such that y[id] == x,  assuming that exact equality is well defined (see double override). \param x The value to be searched for \param y The vector to search through  \returns A SearchResult indicating if the element has been found, and the index it can be found at

	template<typename T, internal::SearchableRange R>
    requires std::convertible_to<T, std::ranges::range_value_t<R>>
	inline SearchResult find(const T & x, const R & y)
	{
		auto it = std::find(std::begin(y),std::end(y),x);

		if (it != std::end(y))
		{
			return {true,static_cast<size_t>(std::distance(std::begin(y),it))};
		}
		else
		{
			return {false,NotFound};
		}
	}
	

	//!Gets first id such that (y[id]- x) < tolerance. \param x The value to be searched for \param y The vector to search through \param tolerance The fractional difference permitted between two double values for them to be declared "approximately equal" \returns The index of the first element in the array which matches x. Returns JSL::NotFound (equivalent to string::npos) if no match found
	SearchResult find(double x, const std::vector<double> & y, double tolerance);

	template<typename T, internal::SearchableRange R>
    requires std::convertible_to<T, std::ranges::range_value_t<R>>
	inline bool contains(const T & x, const R & y)
	{
		return static_cast<bool>(find(x, y));
	}
   
}