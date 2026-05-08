#include <JSL.h>
#include <cmath>
namespace JSL
{
    using coordset = std::vector<Coordinate>;
 
    struct Limits
    {
        Coordinate Lower = {INT32_MAX,INT32_MAX};
        Coordinate Upper = {-INT32_MAX,-INT32_MAX};
        Limits(coordset input)
        {
            for (auto point : input)
            {
                if (point.X < Lower.X)
                {
                    Lower.X = point.X;
                }
                if (point.X  >  Upper.X)
                {
                    Upper.X = point.X;
                }
                if (point.Y < Lower.Y)
                {
                    Lower.Y = point.Y;
                }
                if (point.Y  >  Upper.Y)
                {
                    Upper.Y = point.Y;
                }

            }
            double rx = Upper.X - Lower.X;
            double ry = Upper.Y - Lower.Y;

            int scalex = std::round(std::log10(rx));
            int scaley = std::round(std::log10(ry));
            LOG(INFO) << scalex << " " << scaley;
            Upper.X = std::ceil(Upper.X * pow(10,-scalex))*pow(10,scalex);
            Lower.X = std::floor(Lower.X * pow(10,-scalex))*pow(10,scalex);
            Upper.Y = std::ceil(Upper.Y * pow(10,-scaley))*pow(10,scaley);
            Lower.Y = std::floor(Lower.Y * pow(10,-scaley))*pow(10,scaley);
        }
        std::pair<double,double> Fraction(Coordinate P)
        {
            double x = (P.X - Lower.X)/(Upper.X - Lower.X);
            double y = (P.Y - Lower.Y)/(Upper.Y - Lower.Y);
            return {x,y};
        }
    };
    // struct Tics
    // {
    //     std::vector<std::pair<
    // }

    void TextPlot( std::vector<Coordinate> coordinates)
    {
        if (!Terminal::IsANSICapable())
        {
            internal::FatalError("Not in terminal",JSL_LOCATION) << "Text plotting must take place in a tty-terminal";
        }

        int width = 50;
        int height = 25;
        Limits lim(coordinates);
        std::vector<std::vector<std::string>> characterGrid(height,std::vector<std::string>(width," ")); //needs to be a string so can also hold format characters

        for (auto point : coordinates)
        {
            auto [fx,fy] = lim.Fraction(point);
            int cx = fx * (width-1) + 0.49;  
            int cy = fy * (height-1) + 0.49;  
            characterGrid[cy][cx] = JSL::Format::Cyan() + "*";
        }
        LOG(INFO) << lim.Lower.X << " " << lim.Lower.Y << " " << lim.Upper.X << " " << lim.Upper.Y;
        //processing
        for (int r = height-1; r >=0; --r)
        {
            std::cout << JSL::Format::White() << "║";

            for (int c = 0; c < width; ++c)
            {
                std::cout << characterGrid[r][c];
            }
            
            std::cout << "\n";
        }
        std::cout <<  JSL::Format::White() << "╚";
        for (int c = 0; c < width; ++c)
        {
            std::cout << "═";
        }
        std::cout << std::endl;

    }
}