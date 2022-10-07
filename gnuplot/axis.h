#pragma once
#include <string>
#include <iostream>
#include "../FileIO/FileIO.h"
#include "PlotData.h"
#include <math.h>
#include "../System/System.h"
#include "colorArray.h"
namespace JSL
{
	namespace Fonts
	{
		//!A Font::Target enumerates the possible fontsizes which can be set by the gnuplot regime. Used in either the Axis::SetFontSize function or gnuplot::SetFontSize
		enum Target {Global,SuperTitle,Title,Label,Legend};

		//! Generates the appropriate font-resizing string (if size < 0, uses the default Global value) \param size The font size to turn into a string
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

	/*!
		An axis is a single instance of a plotting `grid' on the screen. Multiple such axes can exist simultaneously (via gnuplot::SetMultiplot()), with each axis tracking their own data and properties individually.
	*/
	class Axis
	{
		public:
			bool Lock = false;
			//! Default, empty constructor (necessary for std::vector<Axis> to exist, even if never called)
			Axis(){};

			//! Expected constructor, generates a random id for itself, and generates a temporary directory in which it stores its data \param rootDir The temporary folder generated by JSL::gnuplot, into which all temp data goes
			Axis(std::string rootDir,ColourArray colors)
			{
				DataDir = rootDir + "/" + "axis_" + std::to_string(rand());
				mkdir(DataDir);
				title = "__null__";
				xlabel = "x";
				ylabel = "y";
				DataIdx = 0;
				Colours = colors;
			}

			//! Adds a line plot to the axis, plotting x against y, and using the args to define what the line looks like \param x The x coordinates of the data to be plotted \param y The y coordinates of the data t be plotted, must be the same length as x \param args A variadic list of NameValuePair objects, used to define the properties of the line \returns A reference to the generated JSL::PlotData object, allowing for post-facto modification of the linestyle.
			template<class T, class S, typename... Ts>
			PlotData & Plot(const std::vector<T> & x,const std::vector<S>  & y, NameValuePair<Ts>... args)
			{
				Assert("Scatter only works on vectors of equal size",x.size()==y.size());
				std::string name = NewData();
				writeMultiVectorToFile(name,", ",x,y);

				Data.push_back(PlotData(name,DataIdx,Line,args...));

				return Data[Data.size()-1];
			};

			//! Adds a scatter plot to the axis, plotting x against y, and using the args to define what the line looks like \param x The x coordinates of the data to be plotted \param y The y coordinates of the data t be plotted, must be the same length as x \param args A variadic list of NameValuePair objects, used to define the properties of the line \returns A reference to the generated JSL::PlotData object, allowing for post-facto modification of the linestyle.
			template<class T, class S, typename... Ts>
			PlotData & Scatter(const std::vector<T> & x,const std::vector<S>  & y, NameValuePair<Ts>... args)
			{
				Assert("Scatter only works on vectors of equal size",x.size()==y.size());
				std::string name = NewData();


				writeMultiVectorToFile(name,", ",x,y);

				Data.push_back(PlotData(name,DataIdx,ScatterPoint,args...));

				return Data[Data.size()-1];
			};
			

			template<class T, class S, typename... Ts>
			PlotData & Chart(const std::vector<T> & x,const std::vector<S>  & y, NameValuePair<Ts>... args)
			{
				Assert("Bar Chart only works on vectors of equal size",x.size()==y.size());
				std::string name = NewData();


				std::vector<std::string> mock_x(x.size());
				for (int i = 0; i < x.size(); ++i)
				{
					std::ostringstream os;
					os << "\"" << x[i] << "\"";
					mock_x[i] = os.str();
				}
				std::vector<int> idx(x.size());
				std::iota(idx.begin(),idx.end(),1);
				writeMultiVectorToFile(name," ",idx,mock_x,y);

				Data.push_back(PlotData(name,DataIdx,BarChart,args...));

				return Data[Data.size()-1];
			};

	

			//! The command which generates the gnuplot scripting code for plotting the data on this axis \returns The string corresponding to the gnuplot script, must be written to file to be actionable
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
				SpanSetter();
				TimeSetter("x",isTime_x);
				TimeSetter("y",isTime_y);
				AddProperty("set xlabel \"" + xlabel + "\"" + Fonts::SizeString(axisFontSize));
				AddProperty("set ylabel \"" + ylabel + "\"" + Fonts::SizeString(axisFontSize));
				AddProperty("set tics " + Fonts::SizeString(axisFontSize));
				AddProperty("set style fill solid");
				AddProperty("set boxwidth 0.75");
				LogSetter("x",isLog_x);
				LogSetter("y",isLog_y);
				AngleSetter("x",xTicAngle);
				AngleSetter("y",yTicAngle);
				std::string grid_cmd = "set grid";
				if (!gridActive)
				{
					grid_cmd = "un" + grid_cmd;
				}
				AddProperty(grid_cmd);
				std::string key_cmd = "set key";
				
				if (!legendActive)
				{
					key_cmd = "un" + key_cmd;
				}
				else
				{
					key_cmd += " " + legendLocation;
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

			//! Simple setter for range_x \param min The lower bound of the x range \param max The upper bound of the x range
			void SetXRange(double min, double max)
			{
				range_x = {min,max};
			}
			//! Simple setter for Axis::range_y \param min The lower bound of the y range \param max The upper bound of the y range
			void SetYRange(double min, double max)
			{
				range_y = {min,max};
			}
			//! Simple setter for Axis::xlabel
			void SetXLabel(std::string xl)
			{
				xlabel = xl;
			}
			//! Simple setter for Axis::ylabel
			void SetYLabel(std::string yl)
			{
				ylabel = yl;
			}
			//! Setter for Axis::xlabel which also sets the font size of the axis text
			void SetXLabel(std::string xl, int fontsize)
			{
				xlabel = xl;
				axisFontSize = fontsize;
			}
			//! Setter for Axis::ylabel which also sets the font size of the axis text
			void SetYLabel(std::string yl, int fontsize)
			{
				ylabel = yl;
				axisFontSize = fontsize;
			}
			//! Simple setter for Axis::isLog_x
			void SetXLog(bool val)
			{
				isLog_x = val;
			}
			//! Simple setter for Axis::isLog_y
			void SetYLog(bool val)
			{
				isLog_y = val;
			}
			//! Simple setter for Axis::isTime_x
			void SetXTime(bool val)
			{
				isTime_x = val;
			}
			//! Simple setter for Axis::isTime_y
			void SetYTime(bool val)
			{
				isTime_y = val;
			}
			//! Simple setter for Axis::xTicAngle
			void SetXTicAngle(int val)
			{
				xTicAngle = val;
			}
			//! Simple setter for Axis::yTicAngle;
			void SetYTicAngle(bool val)
			{
				yTicAngle=val;
			}
			//! Simple setter for Axis::legendActive
			void SetLegend(bool state)
			{
				legendActive = state;
			}
			void SetLegendLocation(std::string loc)
			{
				legendLocation = loc;
			}
			//! Simple setter for Axis::title
			void SetTitle(std::string tit)
			{
				title = tit;
			}//! Setter for Axis::title which also sets the fontsize for the title
			void SetTitle(std::string tit, int size)
			{
				title = tit;
				titleFontSize = size;
			}
			//! Setter for Axis::gridActive
			void SetGrid(bool state)
			{
				gridActive = state;
			}
			void SetSpan(double y, double x)
			{
				xSpan = x;
				ySpan = y;
			}
			//!Sets the fontsize of one of the texts associated with the axis \param target The identifier of the text to be changed \param size The desired fontsize
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

			void SetColourMap(std::vector<std::vector<double>> rgb)
			{
				Colours.SetColours(rgb);
			}
			void SetColourMap(int n, std::vector<double> start, std::vector<double> end)
			{
				Colours.SetColours(n,start,end);
			}
			void GlobalColourSet(const ColourArray & c)
			{
				Colours = c;
			}
		private:
			std::string title; //!< The text which appears above the axis 
			std::string xlabel;//!< The text which appears underneath the x axis
			std::string ylabel; //!<The text which appears to the left of the y axis
			std::vector<double> range_x; //!< A (max) length-2 vector detailing the visual range of the plot on the x axis. If length is zero, uses gnuplot auto-scaling
			std::vector<double> range_y;//!< A (max) length-2 vector detailing the visual range of the plot on the x axis. If length is zero, uses gnuplot auto-scaling
			bool isLog_x = false;//!< If true, uses logarithmic scaling on the x axis
			bool isTime_x = false;//!<If true, interprets the x axis as a temporal coordinate
			bool isTime_y = false;//!<If true, interprets the y axis as a temporal coordinate
			bool isLog_y = false;//!< If false, uses logarithmic scaling on the x axis
			bool gridActive = false;//!<If true, overlays a grid onto the plot
			double xSpan = -1;
			double ySpan = -1;
			int xTicAngle = 0; //!<RotationAngle of xtics
			int yTicAngle = 0;//!<Rotation Angle of ytics
			int axisFontSize = -1; //!< The font size used to write both the Axis::xlabel and Axis::ylabel (cannot be different for x/y). If < 0, uses the value of gnuplot::globalFontSize
			int titleFontSize = -1;//!<The font size used to write Axis::title. If < 0, uses the value of gnuplot::globalFontSize
			int legendFontSize = -1;//!<The font size used to write the legend, if active. If < 0, uses the value of gnuplot::globalFontSize
			std::string legendLocation = "left top";
			std::string DataDir; //!< The directory reserved for this axis to write its data to file for gnuplot to later scoop up
			std::vector<PlotData> Data; //!< A vector of data detailing what should be plotted, and what it looks like, accessed during Axis::Show()
			bool legendActive = false; //!< If true, shows a legend on the axis
			int DataIdx; //!< The current counter for how many plots/lines have been added to the axis, used to index Axis::Data
			ColourArray Colours;
			std::string WriteCommand;//!< The string into which the Axis::Show() function puts all its data
			
			//!Called when data added to the axis. Increments the data counter, and generates a filename and file into which the plot data will be saved \returns The name of the file generated to store the data
			std::string NewData()
			{
				++DataIdx;
				std::string name = DataDir + "/dataChunk_" + std::to_string(DataIdx) + ".dat";
				initialiseFile(name);
				return name;
			}

			//!Used to add unique properties to Axis::WriteCommand during the Axis::Show() phase, followed by a linebreak \param line A property to be written to file
			void AddProperty(std::string line)
			{
				WriteCommand += line + "\n";
			}

			//!Called during Axis::Show(), writes the data associated with the i-th line to the Axis::WriteCommand \param i The index of the data to be written
			void AddPlot(int i)
			{
				WriteCommand += Data[i].Write(Colours);
			};

			//!Generates a string for either the x or y axis value of the logarithm toggle, which is then piped to Axis::AddProperty \param prefix Either "x" or "y" \param active Either value of Axis::isLog_x or Axis::isLog_y
			void LogSetter(const std::string & prefix, bool active)
			{
				std::string t = "set log " + prefix;
				if (!active)
				{
					t = "un" + t;
				}
				AddProperty(t);
			}

			void SpanSetter()
			{
				if (xSpan > 0 && ySpan > 0)
				{
					AddProperty("set size " + std::to_string(xSpan) + "," + std::to_string(ySpan));
				}
			}
			//!Generates a string for either the x or y axis value of the range, which is then piped to Axis::AddProperty \param prefix Either "x" or "y" \param active Either value of Axis::xrange or Axis::yrange
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
			void TimeSetter(const std::string & axisPrefix, bool timeActive)
			{
				if (timeActive)
				{
					std::string cmd = "set " + axisPrefix + "data time\nset timefmt \"%s\"";
					AddProperty(cmd);
				}
			}
			void AngleSetter(const std::string & axisPrefix, int angle)
			{
				if (abs(angle) > 0)
				{
					std::string cmd = "set " + axisPrefix + "tics rotate by " + std::to_string(angle);
					if (angle > 0)
					{
						double xOffset = -2 * std::cos(3.141592654/180 * angle);
						double yOffset = -2 * std::sin(3.141592654/180 * angle);
						cmd += " offset " + std::to_string(xOffset) + "," + std::to_string(yOffset);
					}
					// std::cout << cmd << std::endl;
					AddProperty(cmd);
				}
			}
	};
};