#pragma once

namespace JSL
{

	std::vector<double> rgb_to_hsv(std::vector<double> rgb)
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
	std::vector<double> hsv_to_rgb(std::vector<double> hsv)
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


	class ColourArray
	{
		public:
		
			std::vector<double> GetNextColour()
			{
				++Index;
				return ColourList[(Index-1) % ColourList.size()];
				prevColour = "black";
			}
			std::string HoldColour()
			{
				return prevColour;
			}
			ColourArray()
			{
				Index = 0;
				ColourList = {      {0, 0.447, 0.741},
         							{0.85, 0.325,0.098},
			        				{0.929,  0.694 ,0.125},
 	       							{0.494, 0.184 ,  0.556},
        							{0.466, 0.674,  0.188},
        							{0.301, 0.745, 0.933},
        							{0.635, 0.078,0.184}
				};
			}
			void Save(std::string color)
			{
				prevColour = color;
			}
			
			void SetColours(std::vector<std::vector<double>> rgbArray)
			{
				ColourList.resize(rgbArray.size());
				for (int i =0; i < rgbArray.size(); ++i)
				{
					int n = rgbArray.size();
					JSL::Assert("RGB arrays must have length 3", n == 3);
					for (int j = 0; j < n; ++i)
					{
						JSL::Assert("This function accepts only rgb values in the range 0 <= x <= 1", rgbArray[i][j] >= 0, rgbArray[i][j] <= 1);
					}
					ColourList[i] = rgbArray[i];
				}
			}

			void SetColours(int n, std::vector<double> start, std::vector<double> end)
			{
				ColourList.resize(n);
				JSL::Assert("RGB arrays must be of length 3",start.size()==3,end.size()==3);
				auto hsvStart = rgb_to_hsv(start); 
				auto hsvEnd = rgb_to_hsv(end);
				
				for (int i = 0; i < n; ++i)
				{
					double h = hsvStart[0] + (double)i/(n-1) * (hsvEnd[0] - hsvStart[0]); 
					double s = hsvStart[1] + (double)i/(n-1) * (hsvEnd[1] - hsvStart[1]); 
					double v = hsvStart[2] + (double)i/(n-1) * (hsvEnd[2] - hsvStart[2]);
					ColourList[i] = hsv_to_rgb({h,s,v});
				}

				// exit(10);
			}
		private:
			int Index;
			std::vector<std::vector<double>> ColourList;
			std::string prevColour;
	};


}