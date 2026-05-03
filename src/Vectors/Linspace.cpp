#include <JSL/Vectors/Linspace.h>

std::vector<double> JSL::Linspace(double begin, double end, size_t resolution)
{
    double delta = (end - begin)/(resolution-1);
    std::vector<double> out(resolution);

    for (size_t i = 0; i < resolution; ++i)
    {
        out[i] = delta * i + begin;
    }
    out.back() = end; // ensure the end is exactly included
    return out;
}