#pragma once
#include <string>
namespace JSL
{
	enum PlotType {Line,ScatterPoint};
	enum Property {Colour,PenSize,PenType};
	enum LineType {Solid, Dash, DashDot,Dotted, DashDotDot};
	enum ScatterType {Dot, Plus,Cross,Star,OpenSquare,FilledSquare,OpenCircle,FilledCircle,OpenDelta,FilledDelta,OpenNabla,FilledNabla,OpenDiamond,FilledDiamond};
	
	template<typename T>
	struct NameValuePair
	{
		Property Name;
		T Value;
		
		NameValuePair(Property name, T value)
		{
			Name = name;
			Value = value;
		}
	};
	namespace LineProperties
	{
		template<typename T>
		NameValuePair<T> PenSize(T w)
		{
			return NameValuePair<T>(JSL::PenSize,w);
		}
		template<typename T>
		NameValuePair<T> Colour(T c)
		{
			return NameValuePair<T>(JSL::Colour,c);
		}
		template<typename T>
		NameValuePair<T> PenType(T c)
		{
			return NameValuePair<T>(JSL::PenType,c);
		}
		template<typename T>
		NameValuePair<T> ScatterType(T c)
		{
			return NameValuePair<T>(JSL::PenType,c);
		}
	}

	class PlotData
	{
		public:	
			
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
			template<typename T>
			void SetColour(T v)
			{
				Error("Cannot parse colours in the provided format. Please use a supported colour encoding");
			}
			void SetColour(std::string c)
			{
				colour = "\"" + c + "\"";
			}
			void SetColour(const char * c)
			{
				SetColour((std::string) c);
			}
			void SetColour(int r, int g, int b)
			{
				int combined = 65536 * (r % 256) + 256 * (g%256) + (b%256);
				colour = " rgb " + std::to_string(combined);
			}		
			void SetColour(std::vector<int> cs)
			{
				Assert("RGB Vectors must have length 3", cs.size() == 3);
				SetColour(cs[0],cs[1],cs[2]);
			}

			template<typename T>
			void SetPenSize(T v)
			{
				Error("Cannot parse pensizes in the provided format. Please use a supported pensize encoding");
			}
			void SetPenSize(int l)
			{
				penSize = l;
			}	

			template<typename T>
			void SetPenType(T v)
			{
				Error("Cannot parse pensizes in the provided format. Please use a supported pensize encoding");
			}
			void SetPenType(std::string t)
			{
				dashType = t;
			}
			void SetPenType(const char * t)
			{
				dashType = (std::string) t;
			}
			void SetPenType(int i)
			{
				dashType = std::to_string(i);
			}
			void SetPenType(JSL::LineType t)
			{
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
			void SetPenType(JSL::ScatterType t)
			{
				dashType = std::to_string((int)t);
			}
			template<typename T>
			void SetScatterType(T t)
			{
				SetPenType(t);
			}
			std::string Write()
			{
				std::string line = "";
				line += "\t\"" + DataLocation + "\" using 1:2 ";
				switch (type)
				{
					case Line:
					{
						line += "with line ";
						line += "dt " + dashType;
						break;
					}
					case ScatterPoint:
					{
						line += " pt " + dashType;
					}
					default:
					{
						break;
					}

				}
				line += " lw " + std::to_string(penSize);
				if (colour != "__null__")
				{
					line += " lc " + colour;
				}
				line += " title \"" + legend+ "\"";
				return line + ",\\\n";
			}
		private:
			std::string DataLocation;
			int Idx;
			std::string dashType;
			PlotType type;
			std::string legend;
			std::string colour;
			int penSize;
			void DefaultInit()
			{
				
				colour = "__null__";
				dashType = "solid";
				penSize = 1;
				legend = "Data " + std::to_string(Idx);
			}
			template<typename T, typename... Ts>
			void ParseLoop(NameValuePair<T> n1, NameValuePair<Ts>... ns)
			{
				ParseNVPair(n1);
				if constexpr (sizeof...(ns) > 0)
				{
					ParseLoop(ns...);
				}
			}
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
					SetPenType(nv.Value);
					break;
				default:
					break;
				}
			}
	};
}