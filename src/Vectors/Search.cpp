#include <JSL/Vectors/Search.h>

JSL::SearchResult find(double x, const std::vector<double> & y, double tolerance)
{
    auto it = std::find_if(y.begin(), y.end(), [&](double a) {
            return std::abs(x - a) <= tolerance;
    });
    if (it != y.end())
    {
        return {true,static_cast<size_t>(std::distance(y.begin(),it))};
    }
    else
    {
        return {false,JSL::NotFound};
    }
}