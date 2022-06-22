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
				axisCount = xCount * yCount;

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
			template<class T, class S, typename... Ts>
			PlotData & Scatter(const std::vector<T> & x,const std::vector<S>  & y,NameValuePair<Ts>... args)
			{
				return Axes[axis_y][axis_x].Scatter(x,y,args...);	
			};
			template<class T, class S, typename... Ts>//allows templating for non-vector objects which can nevertheless be successfully cast as vectors
			PlotData & Scatter(const T & x,const S  & y,NameValuePair<Ts>... args)
			{
				return Axes[axis_y][axis_x].Scatter((std::vector<double>)x,(std::vector<double>)y,args...);			
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
			void SetXRange(double min,double max)
			{
				Axes[axis_y][axis_x].SetXRange(min,max);
			}
			void SetYRange(double min, double max)
			{
				Axes[axis_y][axis_x].SetYRange(min,max);
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
			void SetXLabel(std::string xl,int size)
			{
				Axes[axis_y][axis_x].SetXLabel(xl, size);
			}
			void SetYLabel(std::string yl, int size)
			{
				Axes[axis_y][axis_x].SetYLabel(yl,size);
			}
			void SetLegend(bool state)
			{
				Axes[axis_y][axis_x].SetLegend(state);
			}
			void WindowSize(int width, int height)
			{
				defaultSize = false;
				windowWidth = width;
				windowHeight = height;
			}
			void SetTitle(std::string tit)
			{
				Axes[axis_y][axis_x].SetTitle(tit);
			}
			void SetTitle(std::string tit, int size)
			{
				Axes[axis_y][axis_x].SetTitle(tit, size);
			}
			void SetSuperTitle(std::string tit)
			{
				superTitle = tit;
			}
			void SetSuperTitle(std::string tit, int size)
			{
				superTitle = tit;
				superTitleFontSize = size;
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
			void SetFontSize(Fonts::Target target, unsigned int size)
			{
				switch (target)
				{
				case Fonts::Global:
					globalFontSize = size;
					break;
				case Fonts::Title: //guard against title being used in single tile mode?
					if (axisCount == 1 && superTitleFontSize < 0)
					{
						superTitleFontSize = size;
					}
					Axes[axis_y][axis_x].SetFontSize(target,size);
					break;
				case Fonts::SuperTitle:
					superTitleFontSize = size;
					break;
				default:
					Axes[axis_y][axis_x].SetFontSize(target,size);
					break;
				}
			}
		private:
			std::string superTitle = "__null__";
			std::vector<std::vector<Axis>> Axes;
			std::string DirName;
			int axis_x;
			int axis_y;
			int axisCount = 1;
			std::string terminal = "qt";
			std::string output = "__null__";
			std::string font = "Geneva";
			int globalFontSize = 14;
			int superTitleFontSize = -1;
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
				introMessage += " font \"" + font  + "," + std::to_string(globalFontSize) +"\""; //needed because mac hates me
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
				if (superTitle != "__null__")
				{
					multiplotInfo += " title \"" + superTitle + "\"" + Fonts::SizeString(superTitleFontSize);
					multiplotInfo += "\n";
				}
				// multiplotInfo += "\\\n";
				// multiplotInfo += "\tmargins 0.1,0.98,0.1,0.98 \\\n\t spacing 0.08,0.08\n\n";
				writeStringToFile(gpFile,multiplotInfo);
			}
	};
}