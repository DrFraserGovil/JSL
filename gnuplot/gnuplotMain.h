#pragma once
#include <vector>
#include "axis.h"
#include "../FileIO/FileIO.h"
#include "../System/System.h"
namespace JSL
{
	//! The global interface for the plotting system. All calls should be piped through this individual class. Each gnuplot object corresponds to a single plot (though they might have severable axes within each plot)
	class gnuplot
	{
	public:
		//! Constructor function, generates a unique name for itself, and creates a directory into which it stores its own data
		gnuplot()
		{
			DirName = "gnuplot_tmp_" + std::to_string(rand());
			Axes = {{Axis(DirName)}};
			axis_x = 0;
			axis_y = 0;
			mkdir(DirName);
		};

		//! Custom destructor which also calls CleanupTempFiles, tidying up after itself
		~gnuplot()
		{
			CleanupTempFiles();
		}

		//! Puts gnuplot into multiplot mode, with yCount rows and xCount columns. Whenever this function is called, the entire set of axes is wiped clean \param yCount The number of rows \param xCount
		void SetMultiplot(int yCount, int xCount)
		{
			Axes = std::vector<std::vector<Axis>>(yCount);
			for (int i = 0; i < yCount; ++i)
			{
				Axes[i].resize(xCount);
				for (int j = 0; j < xCount; ++j)
				{
					Axes[i][j] = Axis(DirName);
				}
				//  = std::vector<Axis>(xCount, DirName);
			}
			axisCount = xCount * yCount;
			axis_x_max = xCount - 1;
			axis_y_max = yCount - 1;
		}
		//! Sets the current axis focus to the y-th row and x-th column. The "axis in focus" is acted upon whenever the gnuplot object is given a command (i.e. gnuplot::Plot) which does not otherwise specify the axis. By "switching focus", calling gnuplot::Plot will write on different axes. It is also possible to act on the axes directly, using the operator[] overload, i.e. gnuplot[i][j].Plot() would produce the same output as gnuplot.SetAxis(i,j); Plot() - though with the second version, a subsequent call to Scatter() would be written to the same axis, which would not be guaranteed for the first case.
		void SetAxis(unsigned int y, unsigned int x)
		{
			Assert("Must index into existing multiplot values", y <= axis_y_max && x <= axis_x_max);
			axis_x = x;
			axis_y = y;
		};
		//! Sets the current axis focus to the idx-th axis, numbering columns-then-rows
		void SetAxis(int idx)
		{
			Assert("Must index into existing multiplot values", idx < axisCount);
			axis_y = idx / Axes[0].size();
			axis_x = idx % Axes[0].size();
		};

		//! Passes the arguments along to the Axis::Plot() function associated with the current axis focus\returns A reference to the generated JSL::PlotData object, allowing for post-facto modification of the linestyle.
		template <class T, class S, typename... Ts>
		PlotData &Plot(const std::vector<T> &x, const std::vector<S> &y, NameValuePair<Ts>... args)
		{
			return Axes[axis_y][axis_x].Plot(x, y, args...);

		};
		//! As with Plot(const std::vector<T> & x,const std::vector<S>  & y, NameValuePair<Ts>... args), but permits non-vector objects which can still be cast into vectors (i.e. JSL::Vector objects)\returns A reference to the generated JSL::PlotData object, allowing for post-facto modification of the linestyle.
		template <class T, class S, typename... Ts>
		PlotData &Plot(const T &x, const S &y, NameValuePair<Ts>... args)
		{
			return Axes[axis_y][axis_x].Plot((std::vector<double>)x, (std::vector<double>)y, args...);
		};
		//! Passes the arguments along to the Axis::Scatter() function associated with the current axis focus \returns A reference to the generated JSL::PlotData object, allowing for post-facto modification of the linestyle.
		template <class T, class S, typename... Ts>
		PlotData &Scatter(const std::vector<T> &x, const std::vector<S> &y, NameValuePair<Ts>... args)
		{
			return Axes[axis_y][axis_x].Scatter(x, y, args...);
		};
		//! As with Scatter(const std::vector<T> & x,const std::vector<S>  & y, NameValuePair<Ts>... args), but permits non-vector objects which can still be cast into vectors (i.e. JSL::Vector objects) \returns A reference to the generated JSL::PlotData object, allowing for post-facto modification of the linestyle.
		template <class T, class S, typename... Ts> // allows templating for non-vector objects which can nevertheless be successfully cast as vectors
		PlotData &Scatter(const T &x, const S &y, NameValuePair<Ts>... args)
		{
			return Axes[axis_y][axis_x].Scatter((std::vector<double>)x, (std::vector<double>)y, args...);
		};

		//! The key function - until this function is called, the plot is hypothetical. Calling this function generates a gnuplot script file, which each axis writes to to generate the appropriate plot, which is then called using a system command, producing the plot.
		void Show()
		{
			std::string gpFile = DirName + "/" + "plotter.gp";
			InitialiseOutput(gpFile);

			if (Axes.size() > 0 || Axes[0].size() > 0)
			{
				WriteMultiplotToFile(gpFile);
			}
			for (int i = 0; i < Axes.size(); ++i)
			{
				for (int j = 0; j < Axes[i].size(); ++j)
				{
					writeStringToFile(gpFile, Axes[i][j].Show());
				}
			}

			// try-catch loop means if gnuplot fails, can still cleanup after itself without leaving tempfiles hanging around
			try
			{
				systemCall("gnuplot -p " + gpFile);
			}
			catch (const std::exception &e)
			{
				CleanupTempFiles();
				Error(PlottingError, "The gnuplot command failed. Could not show plot.");
			}
		};

		//! Allows indexing into the Axes without using the SetAxis(int,int) function, accessing the Axis object directly. Note that this returns a vector of axis objects, hence access must be via gunplot[i][j]
		std::vector<Axis> &operator[](int i)
		{
			return Axes[i];
		};

		//! Calls Axis::SetXRange(double,double) on the axis currently in focus.
		void SetXRange(double min, double max)
		{
			Axes[axis_y][axis_x].SetXRange(min, max);
		}
		//! Calls Axis::SetYRange(double,double) on the axis currently in focus.
		void SetYRange(double min, double max)
		{
			Axes[axis_y][axis_x].SetYRange(min, max);
		}
		//! Calls Axis::SetXLog on the axis currently in focus.
		void SetXLog(bool val)
		{
			Axes[axis_y][axis_x].SetXLog(val);
		}
		//! Calls Axis::SetYLog on the axis currently in focus.
		void SetYLog(bool val)
		{
			Axes[axis_y][axis_x].SetYLog(val);
		}
		//! Calls Axis::SetXLabel(std::string) on the axis currently in focus.
		void SetXLabel(std::string xl)
		{
			Axes[axis_y][axis_x].SetXLabel(xl);
		}
		//! Calls Axis::SetYLabel(std::string) on the axis currently in focus.
		void SetYLabel(std::string yl)
		{
			Axes[axis_y][axis_x].SetYLabel(yl);
		}
		//! Calls Axis::SetXLabel(std::string,int) on the axis currently in focus.
		void SetXLabel(std::string xl, int size)
		{
			Axes[axis_y][axis_x].SetXLabel(xl, size);
		}
		//! Calls Axis::SetYLabel(std::string,int) on the axis currently in focus.
		void SetYLabel(std::string yl, int size)
		{
			Axes[axis_y][axis_x].SetYLabel(yl, size);
		}
		//! Calls Axis::SetLegend on the axis currently in focus.
		void SetLegend(bool state)
		{
			Axes[axis_y][axis_x].SetLegend(state);
		}
		//! Calls Axis::SetXTime(bool) on the axis currently in focus.
		void SetXTime(bool val)
		{
			Axes[axis_y][axis_x].SetXTime(val);
		}
		//! Calls Axis::SetYTime(bool) on the axis currently in focus.
		void SetYTime(bool val)
		{
			Axes[axis_y][axis_x].SetYTime(val);
		}
		//! Calls Axis::SetXTime(bool) on the axis currently in focus.
		void SetXTicAngle(int val)
		{
			Axes[axis_y][axis_x].SetXTicAngle(val);
		}
		//! Calls Axis::SetYTime(bool) on the axis currently in focus.
		void SetYTicAngle(int val)
		{
			Axes[axis_y][axis_x].SetYTicAngle(val);
		}
		void SetGrid(bool val)
		{
			Axes[axis_y][axis_x].SetGrid(val);
		}
		//! Changes the window size of the generated plot, measured in pixels
		void WindowSize(int width, int height)
		{
			defaultSize = false;
			windowWidth = width;
			windowHeight = height;
		}

		//! Calls Axis::SetTitle(std::string) on the axis currently in focus
		void SetTitle(std::string tit)
		{
			Axes[axis_y][axis_x].SetTitle(tit);
		}
		//! Calls Axis::SetTitle(std::string,int) on the axis currently in focus
		void SetTitle(std::string tit, int size)
		{
			Axes[axis_y][axis_x].SetTitle(tit, size);
		}

		//! Simple setter for gnuplot::superTitle
		void SetSuperTitle(std::string tit)
		{
			superTitle = tit;
		}
		//! Simple setter for gnuplot::superTitle, and changes the font size it is written to
		void SetSuperTitle(std::string tit, int size)
		{
			superTitle = tit;
			superTitleFontSize = size;
		}

		//! Simple setter for gnuplot::terminal
		void SetTerminal(std::string t)
		{
			terminal = t;
		}
		//! Simple setter for gnuplot::output
		void SetOutput(std::string out)
		{
			output = out;
		}
		//! Simple setter for gnuplot::font
		void SetFont(std::string f)
		{
			font = f;
		}

		//! Sets the fontsize of one of the texts associated with either the global fonts (such as global default or supertitle), or of the axis currently in focus \param target The identifier of the text to be changed \param size The desired fontsize
		void SetFontSize(Fonts::Target target, unsigned int size)
		{
			switch (target)
			{
			case Fonts::Global:
				globalFontSize = size;
				break;
			case Fonts::Title: // guard against title being used in single tile mode?
				if (axisCount == 1 && superTitleFontSize < 0)
				{
					superTitleFontSize = size;
				}
				Axes[axis_y][axis_x].SetFontSize(target, size);
				break;
			case Fonts::SuperTitle:
				superTitleFontSize = size;
				break;
			default:
				Axes[axis_y][axis_x].SetFontSize(target, size);
				break;
			}
		}

	private:
		std::string superTitle = "__null__"; //!< In multiplot mode, the supertitle appears above all other axis titles
		std::vector<std::vector<Axis>> Axes; //!< The array of JSL::Axis objects containing the relevant plot data
		std::string DirName;				 //!< The temporary directory name created for this gnuplot object
		int axis_x;							 //!< The current x position of the axis in focus
		int axis_y;							 //!< The current y position of the axis in focus
		int axis_x_max = 0;					 //!< The maximum permitted value of gnuplot::axis_x
		int axis_y_max = 0;					 //!< The maximum permitted value of gnuplot::axis_y
		int axisCount = 1;					 //!< The current number of axes on the figure
		std::string terminal = "qt 0";		 //!< The current gnuplot terminal mode. See gnuplot documentation for more details
		std::string output = "__null__";	 //!< The current output file that gnuplot will try to save to, if the terminal permits it. If set to __null__, this argument is ignored
		std::string font = "Geneva";		 //!< The current global font used by the terminal
		int globalFontSize = 14;			//! < The global font size, which is used unless otherwise specified by a SetFontSize call
		int superTitleFontSize = -1;		//!< The font size assigned to gnuplot::superTitle. If < 0, uses the value of gnuplot::globalFontSize
		bool defaultSize = true;			//! Toggle indicating that gnuplot::WindowSize() has not been called -- needed because changing window size is gross in gnuplot, so best left alone!
		int windowWidth;					//! Current value of the window Width
		int windowHeight;					//! Current value of the window height

		//! When called, and if the relevant compiler flag is not set, recursively deletes the directory associated with gnuplot::DirName
		void CleanupTempFiles()
		{
			#ifndef GNUPLOT_NO_TIDY
				rm(DirName, true);
			#endif
		}

		//! Prints a default message and begins the setup of the gnuplot script \param outName the name of the gnuplot script
		void InitialiseOutput(std::string outName)
		{
			initialiseFile(outName);
			std::string introMessage = "#This is a temporary gnuplot script constructed by the JSL::gnuplot library\n#It should delete itself after being called. If it has not, then an error has occured. \n#You may delete this file and the directory containing it without problem.\n";

			introMessage += "set terminal " + terminal;
			introMessage += " font \"" + font + "," + std::to_string(globalFontSize) + "\""; // needed because mac hates me
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
			writeStringToFile(outName, introMessage);
		}

		//! Writes the data associated with the multiplot information to file
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
			writeStringToFile(gpFile, multiplotInfo);
		}
	};
}