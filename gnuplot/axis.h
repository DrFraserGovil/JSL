#pragma once
#include <string>
#include <iostream>
#include "../FileIO/FileIO.h"
#include "PlotData.h"
#include "../System/System.h"
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
				AddProperty("set title \"" + Title + "\"");
				AddProperty("set xlabel \"" + xlabel + "\"");
				AddProperty("set ylabel \"" + ylabel + "\"");
				std::string key_cmd = "set key";
				if (!legendActive)
				{
					key_cmd = "un" + key_cmd;
				}
				AddProperty(key_cmd);
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

			void xrange(double min, double max)
			{
				range_x = {min,max};
			}
			void yrange(double min, double max)
			{
				range_y = {min,max};
			}
			void HasLegend(bool state)
			{
				legendActive = state;
			}
		private:
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

			void RangeSetter(const std::string & axisPrefix, const std::vector<double> & range)
			{
				std::string val = "[*:*]";
				if (range.size() == 2)
				{
					val = "[" + std::to_string(range_x[0]) + ":" + std::to_string(range_x[1]) + "]";
				}
				std::string cmd = "set " + axisPrefix + "range " + val;
				AddProperty(cmd);
			}
	};
};