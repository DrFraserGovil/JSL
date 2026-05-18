#pragma once
#include <string_view>
#include <vector> 
#include <ranges>
namespace JSL::Concept
{
	//Define a nice C++20 concept which defines a 'vector type' as any std::vector<T>, but excludes std::string from being too eager
		template<typename T>
		struct is_vector : std::false_type {};
		template<typename T, typename A>
		struct is_vector<std::vector<T, A>> : std::true_type {};
	//end concept
	
	//A concept which defines objects that can be iterated through, and has a 'size' member
	template<typename T>
	concept SearchableRange = 	std::ranges::input_range<T> && 
								requires(T t)
								{
									{ t.size() } -> std::integral;
								};

	template<typename T>
	concept IndexableRange = 	SearchableRange<T> && 
								requires(T t, size_t index)
								{
									t[index];
								};
	
	template<typename T>
	concept NonStringRange = std::ranges::range<T> && 
								!std::convertible_to<T,std::string_view>;



								
	template<typename T>
	concept TupleLike = requires(T t)
					{
						std::tuple_size<T>::value;
						std::apply([](auto&&...){}, t);
					};
	
}