#pragma once
#include <string>
namespace JSL
{
	enum style {Line,ScatterStar};
	class PlotData
	{
		public:
	
			style Style;
			std::string colour;
			std::string DataLocation;

			PlotData(std::string data,style type) : DataLocation(data)
			{
				Style = type;
			};

			std::string Write()
			{
				std::string line = "";
				line += "\t\"" + DataLocation + "\" using 1:2 ";
				switch (Style)
				{
					case Line:
					{
						line += "with line";
						break;
					}
					default:
					{
						break;
					}

				}
				return line + ",\\\n";
			}
		private:
	};
}