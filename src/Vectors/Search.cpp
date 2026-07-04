#include <JSL/Vectors/Search.h>

namespace JSL::Vector
{
    SearchResult find(const std::vector<double> & y, double x, double tolerance)
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
            return {false,NotFound};
        }
    }
}