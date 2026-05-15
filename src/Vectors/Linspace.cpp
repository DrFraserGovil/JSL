#include <JSL/Vectors/Linspace.h>
#include <JSL/internal/error.h>
std::vector<double> JSL::Vector::linspace(double begin, double end, size_t resolution)
{
    if (resolution < 2)
    {
        JSL::internal::FatalError("Bad linspace",JSL_LOCATION) << "A linspace cannot have " << resolution << " elements (must be at least 2)";
    }
    if (std::abs(end-begin) < 1e-14)
    {
        JSL::internal::FatalError("Bad linspace",JSL_LOCATION) << "A linspace must have endpoints which are separated by more than a machine-epsilon";
    }
    double delta = (end - begin)/(resolution-1);
    std::vector<double> out(resolution);

    for (size_t i = 0; i < resolution; ++i)
    {
        out[i] = delta * i + begin;
    }
    out.back() = end; // ensure the end is exactly included
    return out;
}