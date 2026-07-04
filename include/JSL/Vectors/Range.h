#pragma once
#include <JSL/Concepts/ranges.h>
#include <vector>
namespace JSL::Vector
{

	namespace internal
	{
		void verifyRange(double begin, double end, size_t res);
	}

	/*! @brief Generates a vector of doubles evenly spaced on the domain (begin,end)
		@details There is no requirement that ``begin < end``, but we do require ``begin != end``
		@tparam R a container holding doubles
		@param begin The starting element of the sequence
		@param end The final element of the sequence
		@param resolution The number of elements in the output vector
		@return A vector of length ``resolution``
		@throws runtime_error if ``resolution < 2``
		@throws runtime_error if |begin-end| < 1e-14
	*/
	template <Concept::SearchableRange R = std::vector<double>>
		requires std::convertible_to<double, JSL::Concept::RangeInternalType<R>>
	inline R range(double begin, double end, size_t resolution = 100)
	{
		internal::verifyRange(begin, end, resolution);
		double delta = (end - begin) / (resolution - 1);

		R out;
		if constexpr (requires { out.reserve(resolution); })
		{
			out.reserve(resolution);
		}
		for (size_t i = 0; i < resolution - 1; ++i)
		{
			out.insert(out.end(), begin + i * delta);
		}
		out.insert(out.end(), end); // ensure the end is exactly included
		return out;
	}

} // namespace JSL::Vector
