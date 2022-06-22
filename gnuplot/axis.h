#pragma once
#include <string>
#include <iostream>
#include "../FileIO/FileIO.h"
#include "PlotData.h"
#include "../System/System.h"
namespace JSL
{
	namespace Fonts
	{
		enum Target {Global,SuperTitle,Title,Label,Legend};

		inline std::string SizeString(int size)
		{
			std::string out = "";
			if (size > 0)
			{
				out = " font 	\"," + std::to_string(size) + "\"";
			}
			return out;
		}
	}
	namespace internal
	{
		inline std::string signifier(const double & x,int significance)
		{
			std::ostringstream os;
			os << std::setprecision(significance) << x;
			return os.str();
		}
	}
	class Axis
	{
		public:
			std::string title;
			std::string xlabel;
			std::string ylabel;
			std::vector<double> range_x;
			std::vector<double> range_y;
			bool isLog_x = false;
			bool isLog_y = false;
			Axis(){};
			Axis(std::string rootDir)
			{
				DataDir = rootDir + "/" + "axis_" + std::to_string(rand());
				mkdir(DataDir);
				title = "__null__";
				xlabel = "x";
				ylabel = "y";
				DataIdx = 0;
			}

			template<class T, class S, typename... Ts>
			PlotData & Plot(const std::vector<T> & x,const std::vector<S>  & y, NameValuePair<Ts>... args)
			{
				Assert("Scatter only works on vectors of equal size",x.size()==y.size());
				std::string name = NewData();
				WriteData(name,x,y);

				Data.push_back(PlotData(name,DataIdx,Line,args...));

				return Data[Data.size()-1];
			};

			template<class T, class S, typename... Ts>
			PlotData & Scatter(const std::vector<T> & x,const std::vector<S>  & y, NameValuePair<Ts>... args)
			{
				Assert("Scatter only works on vectors of equal size",x.size()==y.size());
				std::string name = NewData();
				WriteData(name,x,y);

				Data.push_back(PlotData(name,DataIdx,ScatterPoint,args...));

				return Data[Data.size()-1];
			};
			

			std::string Show()
			{
				WriteCommand = "\n\n";
				if (title != "__null__")
				{
					AddProperty("set title \"" + title + "\"" + Fonts::SizeString(titleFontSize));
				}
				else
				{
					AddProperty("unset title");
				}

				AddProperty("set xlabel \"" + xlabel + "\"" + Fonts::SizeString(axisFontSize));
				AddProperty("set ylabel \"" + ylabel + "\"" + Fonts::SizeString(axisFontSize));
				AddProperty("set tics " + Fonts::SizeString(axisFontSize));
				LogSetter("x",isLog_x);
				LogSetter("y",isLog_y);

				std::string key_cmd = "set key";
				if (!legendActive)
				{
					key_cmd = "un" + key_cmd;
				}
				AddProperty(key_cmd + Fonts::SizeString(legendFontSize));
				RangeSetter("x",range_x);
				RangeSetter("y",range_y);
				
				if (DataIdx > 0)
				{
					WriteCommand += "plot ";
					for (int i =0; i < DataIdx; ++i)
					{
						AddPlot(i);
					}
				}
				else
				{
					//creates an empty axis - else gnuplot skips them and the alignment gets all messed up
					AddProperty("set xrange [0:1]\nset yrange[0:1]\nset key off\nplot 5\n\n");
				}
				return WriteCommand;
			}

			void SetXRange(double min, double max)
			{
				range_x = {min,max};
			}
			void SetYRange(double min, double max)
			{
				range_y = {min,max};
			}
			void SetXLabel(std::string xl)
			{
				xlabel = xl;
			}
			void SetYLabel(std::string yl)
			{
				ylabel = yl;
			}
			void SetXLabel(std::string xl, int size)
			{
				xlabel = xl;
				axisFontSize = size;
			}
			void SetYLabel(std::string yl, int size)
			{
				ylabel = yl;
				axisFontSize = size;
			}
			void SetXLog(bool val)
			{
				isLog_x = val;
			}
			void SetYLog(bool val)
			{
				isLog_y = val;
			}
			void SetLegend(bool state)
			{
				legendActive = state;
			}
			void SetTitle(std::string tit)
			{
				title = tit;
			}
			void SetTitle(std::string tit, int size)
			{
				title = tit;
				titleFontSize = size;
			}

			void SetFontSize(Fonts::Target target, unsigned int size)
			{
				switch (target)
				{
					case Fonts::Title:
					{
						titleFontSize = size;
						break;
					}
					case Fonts::Label:
					{
						axisFontSize = size;
						break;
					}
					case Fonts::Legend:
					{
						legendFontSize = size;
						break;
					}
					default:
					{
						Error("Cannot set global or supertitle font size from within axis scope");
					}
				}
			}
		private:
			int axisFontSize = -1;
			int titleFontSize = -1;
			int legendFontSize = -1;
			std::string DataDir;
			std::vector<PlotData> Data;
			bool legendActive = false;
			int DataIdx;
			std::string WriteCommand;
			std::string NewData()
			{
				++DataIdx;
				std::string name = DataDir + "/dataChunk_" + std::to_string(DataIdx);
				return name;
			}
			template<typename T, typename S>
			void WriteData(std::string fileName,const std::vector<T> & x,const std::vector<S>  & y)
			{
				initialiseFile(fileName);
				writeMultiVectorToFile(fileName,", ",x,y);
			}
			void AddProperty(std::string line)
			{
				WriteCommand += line + "\n";
			}
			void AddPlot(int i)
			{
				WriteCommand += Data[i].Write();
			};

			void LogSetter(const std::string & prefix, bool active)
			{
				std::string t = "set log " + prefix;
				if (!active)
				{
					t = "un" + t;
				}
				AddProperty(t);
			}
			void RangeSetter(const std::string & axisPrefix, const std::vector<double> & range)
			{
				std::string val = "[*:*]";
				if (range.size() == 2)
				{
					val = "[" + internal::signifier(range[0],8) + ":" + internal::signifier(range[1],8) + "]";
				}


				std::string cmd = "set " + axisPrefix + "range " + val;
				AddProperty(cmd);
			}

			
	};
};