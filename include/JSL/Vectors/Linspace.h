#pragma once

#include <vector>
namespace JSL::Vector
{
	/// @brief Generates a vector of doubles evenly spaced on the domain (begin,end)
	/// @details There is no requirement that ``begin < end``, but we do require ``begin != end``
	/// @param begin The starting element of the sequence
	/// @param end The final element of the sequence
	/// @param resolution The number of elements in the output vector
	/// @return A vector of length ``resolution``
	/// @throws runtime_error if ``resolution < 2``
	/// @throws runtime_error if |begin-end| < 1e-14
	std::vector<double> linspace(double begin, double end, size_t resolution = 100);
} // namespace JSL::Vector
