#pragma once
#include <vector>
#include "axis.h"
#include "../FileIO/FileIO.h"
namespace JSL
{
	class gnuplot
	{
		public:
			gnuplot()
			{
				DirName = "gnuplot_tmp_" + std::to_string(rand());
				axis = Axis(DirName);
				mkdir(DirName);
			};
			~gnuplot()
			{
				CleanupTempFiles();
			}

			void SetMultiplot(int yCount, int xCount)
			{
				subAxes = std::vector<std::vector<Axis>>(yCount);
				for (int i = 0; i < yCount; ++i)
				{
					subAxes[i] = std::vector<Axis>(xCount,DirName);
				}

			}
			void SetAxis(int y, int x)
			{
				axis_x = x;
				axis_y = y;
			};
			void SetAxis(int idx)
			{
				axis_x = idx % subAxes.size();
				axis_y = (idx - axis_x)/(subAxes.size());
				std::cout << axis_y << "  " << axis_x << std::endl;
			};
			template<typename T, typename S>
			void Plot(const std::vector<T> & x,const std::vector<S>  & y)
			{
				if (subAxes.size() == 0)
				{
					axis.Plot(x,y);
				}
				else
				{
					subAxes[axis_y][axis_x].Plot(x,y);
				}			
			};
			template<typename T, typename S>
			void Scatter(const std::vector<T> & x,const std::vector<S>  & y)
			{
				if (subAxes.size() == 0)
				{
					axis.Scatter(x,y);
				}
				else
				{
					subAxes[axis_y][axis_x].Scatter(x,y);
				}	
			};
			void Show()
			{
				std::string gpFile = DirName + "/" + "plotter.gp";
				InitialiseOutput(gpFile);

				if (subAxes.size() == 0)
				{
					writeStringToFile(gpFile,axis.Show());
				}
				else
				{
					WriteMultiplotToFile(gpFile);
					for (int i =0; i < subAxes.size(); ++i)
					{
						for (int j = 0; j < subAxes[i].size(); ++j)
						{
							writeStringToFile(gpFile,subAxes[i][j].Show());
						}
					}
				}
				std::string command = "gnuplot -p " + gpFile;
				std::system(command.c_str());	
			};

			std::vector<Axis> & operator[](int i)
			{
				std::cout << "Indexing " << i << " out of " << subAxes.size() << std::endl;
				if (subAxes.size() == 0)
				{
					if (i != 0)
					{
						std::cout << "ERROR -- you cannot index into subplots before intialising" << std::endl;
						CleanupTempFiles();
						exit(5);
					}
				}
				return subAxes[i];
			};
			void xrange(double min,double max)
			{
				axis.xrange(min,max);
			}
			void yrange(double min, double max)
			{
				axis.yrange(min,max);
			}
		private:
			Axis axis;
			std::vector<std::vector<Axis>> subAxes;
			std::string DirName;
			int axis_x;
			int axis_y;
			void CleanupTempFiles()
			{
				rm(DirName,true);
			}

			void InitialiseOutput(std::string outName)
			{
				initialiseFile(outName);
				std::string introMessage = "#This is a temporary gnuplot script constructed by the JSL::gnuplot library\n#It should delete itself after being called. If it has not, then an error has occured. \n#You may delete this file and the directory containing it without problem.\n";

				introMessage += "set term qt font \"Geneva,9\"\n"; //needed because mac hates me
				// introMessage += "set terminal cairo\n";
				writeStringToFile(outName,introMessage);				

			}
			void WriteMultiplotToFile(std::string gpFile)
			{
				std::string multiplotInfo = "\nset multiplot layout ";
				multiplotInfo += std::to_string(subAxes.size()) + "," + std::to_string(subAxes[0].size());
				multiplotInfo += "\\\n";
				multiplotInfo += "\tmargins 0.1,0.98,0.1,0.98 \\\n\t spacing 0.08,0.08\n\n";
				writeStringToFile(gpFile,multiplotInfo);
			}
	};
}