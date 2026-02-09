#pragma once

#include <vector>
#include <cmath>
namespace JSL::Plotting::Color
{
    //include color converters because hsv interpolation is much nicer than rgb

    inline std::vector<double> Rgb2Hsv(std::vector<double> rgb)
	{
		double r = rgb[0];
		double g = rgb[1];
		double b = rgb[2];
		

		double maxCol = std::max(r,std::max(g,b));
		double minCol = std::min(r,std::min(g,b));
		double v = maxCol;
		double diff = maxCol - minCol;

		
		double h = 0;
		if (diff > 1e-3)
		{
			if (v == r)
			{
				h = 60 * (g-b)/diff;
			}
			if (v == g)
			{
				h = 60* ( 2.0 + (b-r)/diff);
			}
			if (v == b)
			{
				h = 60 * (4.0 + (r-g)/diff);
			}
		}
		double s = 0;
		if (v > 0)
		{
			s = diff/v;
		}
		return {h,s,v};
	}
	inline std::vector<double> Hsv2Rgb(std::vector<double> hsv)
	{
		double h = hsv[0];
		double s = hsv[1];
		double v = hsv[2];
		double r =0,b=0,g=0;

		double c = v * s;
		double hP = h/60;
		double modTerm = h/60 - 2 * floor(h/120);
		double x = c* (1.0 - abs(modTerm - 1));

		switch ((int)hP)
		{
			case 0:
				r = c;	g = x;	b = 0;
				break;
			case 1:
				r = x;	g = c;	b = 0;
				break;
			case 2:
				r = 0;	g = c; 	b = x;
				break;
			case 3:
				r = 0;	g = x; 	b = c;
				break;
			case 4:
				r = x;	g = 0;	b = c;
				break;
			case 5:
				r = c;	g = 0;	b = x;
				break;
		}
		double m = v - c;
		return {r+m,g+m,b+m};
	}



    class Cycle
    {
        public:

        private:
            std::vector<double> 
    };

    
}