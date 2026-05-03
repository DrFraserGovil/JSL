#pragma once
#include <JSL.h>
#include <JSL/Display/Log.h>
#include <ranges>
namespace JSL
{
	namespace internal
	{
		template<typename T>
		concept Plottable = std::ranges::range<T> && 
						requires(T t) {
						{ t.size() } -> std::integral;
					} &&
					std::convertible_to<std::ranges::range_value_t<T>, double>;;
	}

	struct Coordinate
	{
		double X;
		double Y;
	};

	void TextPlot(std::vector<Coordinate> coordinates);

	template<internal::Plottable T, internal::Plottable U>
	void TextPlot(const T & x, const U & y)
	{

		if (x.size() != y.size())
		{
			internal::FatalError("Size mismatch in plot",JSL_LOCATION) << "x (" << x.size() << ") and y (" << y.size() << ") are of different sizes";
		}
		std::vector<Coordinate> coords;
		coords.reserve(x.size());

		auto y_it = std::begin(y);
		for (const auto& xv : x)
		{
			coords.emplace_back(static_cast<double>(xv), static_cast<double>(*y_it));
			++y_it;
		}
		TextPlot(coords);
	}

	
}