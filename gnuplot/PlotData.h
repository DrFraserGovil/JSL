#pragma once
#include <string>
#include "colorArray.h"
namespace JSL
{
	enum PlotType {Line,ScatterPoint,BarChart};
	enum Property {Colour,PenSize,PenType,Legend};
	enum LineType {Solid, Dash, DashDot,Dotted, DashDotDot};
	enum ScatterType {Dot, Plus,Cross,Star,OpenSquare,FilledSquare,OpenCircle,FilledCircle,OpenDelta,FilledDelta,OpenNabla,FilledNabla,OpenDiamond,FilledDiamond};
	
	//! A simple struct for associating a datatype a member of JSL::Property
	template<typename T>
	struct NameValuePair
	{
		Property Name; //!< Informs JSL::PlotData which internal property the Value is associated with
		T Value; //!< An arbitrary datum, which JSL::PlotData will try to interpret using one of its many template functions.
		//!Boring constructor
		NameValuePair(Property name, T value)
		{
			Name = name;
			Value = value;
		}
	};
	namespace LineProperties
	{
		//! Auto constructs pensize specifier
		inline NameValuePair<int> PenSize(int w)
		{
			return NameValuePair<int>(JSL::PenSize,w);
		}
		//! Auto constructs single-argument colour specifier. Has to be template because can use either string or std::vector types to specify
		template<typename T>
		inline NameValuePair<T> Colour(T c)
		{
			return NameValuePair<T>(JSL::Colour,c);
		}
		//! Auto constructs pentype specifier
		inline NameValuePair<LineType> PenType(LineType c)
		{
			return NameValuePair<LineType>(JSL::PenType,c);
		}
		//! Auto constructs scattertype specifier
		inline NameValuePair<JSL::ScatterType> ScatterType(JSL::ScatterType c)
		{
			return NameValuePair<JSL::ScatterType>(JSL::PenType,c);
		}
		//! Auto constructs legend specifier
		inline NameValuePair<std::string>Legend(std::string s)
		{
			return NameValuePair<std::string>(JSL::Legend,s);
		}
	}
	//! A holder class for the datalocation, and line specification for each plot added to an axis.
	class PlotData
	{
		public:	
			//! Constructor \param data The relative filepath to the location where the associated data is stored. \param idx The id of the data (and the index of this object within the Axis::Data vector). \param args A variadic list of JSL::NameValuePair objects used for pre-facto changes to the line style 
			template<typename... Ts>
			PlotData(std::string data,int idx,PlotType t, NameValuePair<Ts>... args) : DataLocation(data)
			{
				Idx = idx;
				type = t;
				DefaultInit();
				if constexpr (sizeof...(args) > 0)
				{	
					ParseLoop(args...);
				}
			}

			//! Default template function - used to throw error messages if innapropriate colour specifications used. **Note the spelling of colour!!**
			template<typename T>
			void SetColour(T v)
			{
				Error("Cannot parse colours in the provided format. Please use a supported colour encoding");
			}
			//! Sets the colour value of the line to the given string. No checks are made that this string corresponds to a valid gnuplot colour.**Note the spelling of colour!!**
			void SetColour(std::string c)
			{
				colour = "\"" + c + "\"";
			}
			//! SetColour override for c-style string - simple converts to std::string then calls SetColour(std::string c) **Note the spelling of colour!!**
			void SetColour(const char * c)
			{
				SetColour((std::string) c);
			}
			//! Sets the colour to that specified by the RGB value, converting it into a gnuplot-readable format **Note the spelling of colour!!** \param r The red value (between 0 and 255) \param g The green value (between 0 and 255) \param b The blue value (between 0 and 255)
			void SetColour(unsigned int r, unsigned int g, unsigned int b)
			{
				int combined = 65536 * (r % 256) + 256 * (g%256) + (b%256); //bitshifting nonsense
				colour = " rgb " + std::to_string(combined);
			}		
			//! Sets the colour to that specified by the RGB value **Note the spelling of colour!!** \param cs A length-three vector corresponding to RGB values
			void SetColour(std::vector<unsigned int> cs)
			{
				Assert("RGB Vectors must have length 3", cs.size() == 3);
				SetColour(cs[0],cs[1],cs[2]);
			}
			void SetColour(std::vector<double> cs)
			{
				Assert("RGB Vectors must have length 3", cs.size() == 3);
				Assert("When passed as doubles, colour vals must be 0<=x<=1",abs(cs[0]) <= 1, abs(cs[1]) <= 1, abs(cs[2]) <=	 1);
				SetColour(255*abs(cs[0]),255*abs(cs[1]),255*abs(cs[2]));
			}
		
			//! Default template function - used to throw error messages if innapropriate pensize specifications used.
			template<typename T>
			void SetPenSize(T v)
			{
				Error("Cannot parse pensizes in the provided format. Please use a supported pensize encoding");
			}

			//! Sets the width of the pen used to draw lines (and also the thickness of scatter-points)
			void SetPenSize(int l)
			{
				penSize = l;
			}	

			//! Default template function - used to throw error messages if innapropriate pen type specification used.
			template<typename T>
			void SetPenType(T v)
			{
				Error("Cannot parse pentypes in the provided format. Please use a supported pensize encoding");
			}

			//! Sets the LineType (dashed, solid, etc) for the object \param t The JSL::LineType specifier
			void SetPenType(JSL::LineType t)
			{
				Assert("Can only set pentype for line objects - use ScatterType for scatterobjects",type==Line || type==BarChart);
				switch (t)
				{
					case Solid:
						dashType = "1";
						break;
					case Dash:
						dashType = "2";
						break;
					case DashDot:
						dashType = "4";
						break;
					case Dotted:
						dashType = "3";
						break;
					case DashDotDot:
						dashType = "5";
						break;
					default:
						dashType = "-1";
						break;

				}
			}
			//! Default template function - used to throw error messages if innapropriate scatter type specification used.
			template<typename T>
			void SetScatterType(T t)
			{
				std::cout << t << std::endl;
				Error("Cannot parse scattertype in the provided format. Please use a supported pensize encoding");
			}

			//! Sets the LineType (circle, dot, square) for the object \param t The JSL::ScatterType specifier
			void SetScatterType(ScatterType t)
			{
				Assert("Can only set scattertype for scatter objects - use LineType for line objects",type==ScatterPoint);
				dashType = std::to_string((int)t);
			}
			//! Default template function - used to throw error messages if innapropriate specification used.
			template<typename T>
			void SetLegend(T t)
			{
				Error("Cannot parse legends as anything other than strings");
			}

			//! Sets the name for this plot on the legend
			void SetLegend(std::string s)
			{
				legend = s;
				useLegend = true;
			}
			void SetLegend(bool t)
			{
				useLegend = t;
			}
			//! Formats the internal data and writes it to a string for use in a gnuplot script \returns A string containing the necessary data to plot the internal data
			std::string Write(ColourArray & colours)
			{
				std::string line = "";
				line += "\t\"" + DataLocation + "\" ";
				switch (type)
				{
					case Line:
					{
						line += "using 1:2 with lines ";
						line += "dt " + dashType;
						break;
					}
					case ScatterPoint:
					{
						line += "using 1:2  pt " + dashType;
						break;
					}
					case BarChart:
					{
						line += "using 1:3:xtic(2) with boxes ";
						break;
					}
					default:
					{
						break;
					}

				}
				line += " lw " + std::to_string(penSize);
				bool holdColour =  colour == "\"hold\"" || colour == "\"previous\"";
				if (holdColour)
				{
					colour = colours.HoldColour();
				}
				if (colour == "__null__")
				{
					SetColour(colours.GetNextColour());
					// std::cout << colour << std::endl;
				}
				colours.Save(colour);
				line += " lc " + colour;
				
				if (!useLegend)
				{
					line += " notitle";
				}
				else
				{
					line += " title \"" + legend+ "\"";
				}
				return line + ",\\\n";
			}
		private:
			std::string DataLocation; //!< The file location for the x/y data used in this plot
			int Idx; //!< The ID of this plot, pretty much only used to generate a default legend (i.e. Plot 1)
			std::string dashType; //!< The dashType specifier (either for linestyle - dash/solid, or scatterstyle - circle/dot/square etc.)
			PlotType type; //!< Denotes either Line plots or scatter plots etc.
			std::string legend; //!< The name associated with this data on the legend
			std::string colour; //!< The colour used to plot the data - if not specified, uses the default gnuplot colour roster.
			int penSize; //!< The thickness of the pen used to write the data
			bool useLegend = false;
			//! Sets the default values of the members
			void DefaultInit()
			{
				
				colour = "__null__";
				
				switch (type)
				{
					case Line:
						SetPenType(Solid);
						break;
					case ScatterPoint:
						SetScatterType(OpenCircle);
						break;
					case BarChart:
						SetPenType(Solid);
						break;
				};

				penSize = 1;
				legend = "Data " + std::to_string(Idx);
			}

			//! Variadic function which parses the JSL::NameValuePairs provided to the function for pre-facto modification
			template<typename T, typename... Ts>
			void ParseLoop(NameValuePair<T> n1, NameValuePair<Ts>... ns)
			{
				ParseNVPair(n1);
				if constexpr (sizeof...(ns) > 0)
				{
					ParseLoop(ns...);
				}
			}
			//! Given a JSL::NameValuePair object, attempt to interpret it, given the Name value, passing the Value portion to the relevant templated function outlined above
			template<typename T>
			void ParseNVPair(NameValuePair<T> nv)
			{
				switch (nv.Name)
				{
				case Colour:
					SetColour(nv.Value);
					break;
				case PenSize:
					SetPenSize(nv.Value);
					break;
				case PenType:
					if (type == Line)
					{
						SetPenType(nv.Value);
					}
					else
					{
						SetScatterType(nv.Value);
					}
					break;
				case Legend:
					SetLegend(nv.Value);
					break;
				
				default:
					break;
				}
			}
	};
}