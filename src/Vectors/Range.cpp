#include <JSL/Vectors/Range.h>
#include <JSL/internal/error.h>
void JSL::Vector::internal::verifyRange(double begin, double end, size_t resolution)
{
	if (resolution < 2)
	{
		JSL::internal::FatalError("Bad linspace", JSL_LOCATION) << "A linspace cannot have " << resolution << " elements (must be at least 2)";
	}
	if (std::abs(end - begin) < 1e-14)
	{
		JSL::internal::FatalError("Bad linspace", JSL_LOCATION) << "A linspace must have endpoints which are separated by more than a machine-epsilon";
	}
}
