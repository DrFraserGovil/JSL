#pragma once
#include <vector>
#include "axis.h"
#include "../FileIO/FileIO.h"
#include "../System/System.h"
namespace JSL
{
	class gnuplot
	{
		public:
			gnuplot()
			{
				DirName = "gnuplot_tmp";
				#ifdef JSL_MULTIPLE_GNUPLOT_INSTANCE
					DirName += "_" + std::to_string(rand());
				#endif
				Axes = {{Axis(DirName)}};
				axis_x = 0;
				axis_y = 0;
				mkdir(DirName);
			};
			~gnuplot()
			{
				CleanupTempFiles();
			}

			void SetMultiplot(int yCount, int xCount)
			{
				Axes = std::vector<std::vector<Axis>>(yCount);
				for (int i = 0; i < yCount; ++i)
				{
					Axes[i] = std::vector<Axis>(xCount,DirName);
				}

			}
			void SetAxis(int y, int x)
			{
				axis_x = x;
				axis_y = y;
			};
			void SetAxis(int idx)
			{
				axis_y = idx  / Axes[0].size();
				axis_x = idx % Axes[0].size();	
			};
			template<class T, class S, typename... Ts>
			PlotData & Plot(const std::vector<T> & x,const std::vector<S>  & y, NameValuePair<Ts>... args)
			{
				return Axes[axis_y][axis_x].Plot(x,y,args...);			
			};
			template<class T, class S,typename... Ts>//allows templating for non-vector objects which can nevertheless be successfully cast as vectors
			PlotData & Plot(const T & x,const S  & y, NameValuePair<Ts>... args)
			{
				return Axes[axis_y][axis_x].Plot((std::vector<double>)x,(std::vector<double>)y, args...);			
			};
			template<class T, class S>
			PlotData & Scatter(const std::vector<T> & x,const std::vector<S>  & y)
			{
				return Axes[axis_y][axis_x].Scatter(x,y);	
			};
			template<class T, class S>//allows templating for non-vector objects which can nevertheless be successfully cast as vectors
			PlotData & Scatter(const T & x,const S  & y)
			{
				return Axes[axis_y][axis_x].Scatter((std::vector<double>)x,(std::vector<double>)y);			
			};

			void Show()
			{
				std::string gpFile = DirName + "/" + "plotter.gp";
				InitialiseOutput(gpFile);

				if (Axes.size() > 0 || Axes[0].size() > 0)
				{
					WriteMultiplotToFile(gpFile);
				}
				for (int i =0; i < Axes.size(); ++i)
				{
					for (int j = 0; j < Axes[i].size(); ++j)
					{
						writeStringToFile(gpFile,Axes[i][j].Show());
					}
				}
				
				systemCall("gnuplot -p " + gpFile);
			};

			std::vector<Axis> & operator[](int i)
			{
				return Axes[i];
			};
			void xrange(double min,double max)
			{
				Axes[axis_y][axis_x].xrange(min,max);
			}
			void yrange(double min, double max)
			{
				Axes[axis_y][axis_x].yrange(min,max);
			}
			void SetXLog(bool val)
			{
				Axes[axis_y][axis_x].SetXLog(val);
			}
			void SetYLog(bool val)
			{
				Axes[axis_y][axis_x].SetYLog(val);
			}
			void SetXLabel(std::string xl)
			{
				Axes[axis_y][axis_x].SetXLabel(xl);
			}
			void SetYLabel(std::string yl)
			{
				Axes[axis_y][axis_x].SetYLabel(yl);
			}
			void WindowSize(int width, int height)
			{
				defaultSize = false;
				windowWidth = width;
				windowHeight = height;
			}

			void SetTerminal(std::string t)
			{
				terminal = t;
			}
			void SetOutput(std::string out)
			{
				output = out;
			}
			void SetFont(std::string f)
			{
				font = f;
			}
			void HasLegend(bool state)
			{
				Axes[axis_y][axis_x].HasLegend(state);
			}
		private:
		
			std::vector<std::vector<Axis>> Axes;
			std::string DirName;
			int axis_x;
			int axis_y;

			std::string terminal = "qt";
			std::string output = "__null__";
			std::string font = "Geneva,9";
			bool defaultSize = true;
			int windowWidth;
			int windowHeight;
			void CleanupTempFiles()
			{
				#ifndef GNUPLOT_NO_TIDY
					rm(DirName,true);
				#endif
			}

			void InitialiseOutput(std::string outName)
			{
				initialiseFile(outName);
				std::string introMessage = "#This is a temporary gnuplot script constructed by the JSL::gnuplot library\n#It should delete itself after being called. If it has not, then an error has occured. \n#You may delete this file and the directory containing it without problem.\n";

				introMessage += "set terminal " + terminal;
				introMessage += " font \"" + font +"\""; //needed because mac hates me
				if (!defaultSize)
				{
					introMessage += " size " + std::to_string(windowWidth) + "," + std::to_string(windowHeight) + "\n";
				}
				introMessage += "\n";
				if (output != "__null__")
				{
					introMessage += "set output \"" + output + "\"\n";
				}
				
				// introMessage += "set terminal cairo\n";
				writeStringToFile(outName,introMessage);				

			}
			void WriteMultiplotToFile(std::string gpFile)
			{
				std::string multiplotInfo = "\nset multiplot layout ";
				multiplotInfo += std::to_string(Axes.size()) + "," + std::to_string(Axes[0].size());
				multiplotInfo += "\\\n";
				multiplotInfo += "\tmargins 0.1,0.98,0.1,0.98 \\\n\t spacing 0.08,0.08\n\n";
				writeStringToFile(gpFile,multiplotInfo);
			}
	};
}