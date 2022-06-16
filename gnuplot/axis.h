#pragma once
#include <string>
#include <iostream>
#include "../FileIO/FileIO.h"
#include "PlotData.h"
namespace JSL
{
	class Axis
	{
		public:
			std::string Title;
			std::string xlabel;
			std::string ylabel;
			std::vector<double> range_x;
			std::vector<double> range_y;
			Axis(){};
			Axis(std::string rootDir)
			{
				DataDir = rootDir + "/" + "axis_" + std::to_string(rand());
				mkdir(DataDir);
				Title = "Test";
				xlabel = "x";
				ylabel = "y";
				DataIdx = 0;
			}

			template<typename T, typename S>
			void Plot(const std::vector<T> & x,const std::vector<S>  & y)
			{
				assert(x.size()==y.size());
				std::string name = NewData();
				WriteData(name,x,y);
				Data.push_back(PlotData(name,Line));
			};

			template<typename T, typename S>
			void Scatter(const std::vector<T> & x,const std::vector<S>  & y)
			{
				assert(x.size()==y.size());
				std::string name = NewData();
				WriteData(name,x,y);
				Data.push_back(PlotData(name,ScatterStar));
			};

			std::string Show()
			{
				WriteCommand = "\n\n";
				AddProperty("set title \"" + Title + "\"");
				AddProperty("set xlabel \"" + xlabel + "\"");
				AddProperty("set ylabel \"" + ylabel + "\"");
				if (range_x.size() == 2)
				{
					AddProperty("set xrange [" + std::to_string(range_x[0]) + ":" + std::to_string(range_x[1]) + "]");
				}
				else
				{
					AddProperty("set xrange [*:*]");
				}
				if (range_y.size() == 2)
				{
					AddProperty("set yrange [" + std::to_string(range_y[0]) + ":" + std::to_string(range_y[1]) + "]");
				}
				else
				{
					AddProperty("set yrange [*:*]");
				}
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
					AddProperty("set xrange [0:1]\nset yrange[0:1]\nset key off\nplot 5\n\n");
				}
				return WriteCommand;
			}

			void xrange(double min, double max)
			{
				range_x = {min,max};
			}
			void yrange(double min, double max)
			{
				range_y = {min,max};
			}
		private:
			std::string DataDir;
			std::vector<PlotData> Data;
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
	};
};