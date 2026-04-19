#pragma once
#include <exception>
#include <mutex>
#include <string>
#include <vector>
#include <stdexcept>
#include <unistd.h> // For isatty()
#include <cstdio>   // For fileno() and stderr
#include "ANSI_Codes.h"
/*!
    An encoding for different levels of logs. Levels are hierarchical: WARN includes ERROR, and INFO includes WARN (and therefore, also ERROR).
	
	Levels are assumed to be numerically increasing. A lower level means more important.
*/
enum LogLevel 
{
    ERROR=0, //!< Level 0. Used to indicate points where the code is throwing errors. 
    WARN=1,  //!< Level 1. Used to indicate where problems were encountered, but a default assumption was made. Also used to indicate `are you sure about this?'
    INFO=2,  //!< Level 2. General progress information.
    DEBUG=3, //!< Level 3. High density of information, likely to bottleneck code. Used for debugging information

	MAXLEVEL //!<Used as an indicator of the 'allowed max log level' - used only for loop checks etc. Should never be assigned to
};

//! Convert integers to LogLevels. @param level an integer between 0 and 3 @throws runtime_error if level is out of bounds @returns The corresponding LogLevel 
inline LogLevel LogLevelConvert(int level)
{
	switch(level){
		case 0: 
			return ERROR; break;
		case 1:
			return WARN;break;
		case 2:
			return INFO;break;
		case 3:
			return DEBUG;break;
		default:
			throw std::runtime_error(std::to_string(level) + "is not a valid logging level");break;
	}
}



namespace JSL::Log
{
	/*!
	\brief A packager for globally accessible variables for the LoggerCore object to refer to. 

	\details A single \c ConfigObject exists, the globally defined \ref Log::Config. Defined by the Initialise_JSL_Log macro call
	*/
	struct ConfigObject
	{
		//! If true, all Log commands have a newline automatically appended to them. Default: TRUE
		bool AppendNewline;
		
		//! If true, all Log commands are preceeded by 'header info' about the type of log. Headers occur only once per LOG call, linebreaks are automatically aligned. Default: TRUE
		bool ShowHeaders; 
		
		//! Any calls to @ref LOG with a value higher than this are ignored. Default: INFO
		LogLevel Level; 

		//! Automatically detected at runtime-start. True if the output stream is a tty terminal capable of interpreting @ref ANSI commands.
		bool TerminalOutput; 

		bool ForceClear = false;

		TerminalFormat WarnColour = JSL::Text::Purple;
		TerminalFormat ErrorColour = JSL::Text::Red;
		TerminalFormat InfoColour = JSL::Text::White;
		TerminalFormat DebugColour = JSL::Text::Blue;

		bool DebugBoxing = true;
		size_t DebugLineSize = 40;

		//! Default initialiser. Initialises TerminalOutput, and sets Level=INFO, ShowHeaders=true and AppendNewline=true.
		ConfigObject()
        {
            SetLevel(INFO);
            ShowHeaders = true;
            AppendNewline = true;
            TerminalOutput = isatty(fileno(stdout));
        }
		
		//! Convenient interface for setting Level from integer values
		void SetLevel(int level)
        {
            Level = LogLevelConvert(level);
        }

		/*!
			Used to initialise globally \ref Config, after the command line arguments have been parsed.
			@param level The (as an integer) to be assigned to the Level
			@param header The value to be assigned to ShowHeaders
			@param welcomeFile The file location of the RAMICES 'welcome.dat' file, which prints a cute little message
		*/ 
		void Initialise(int level,bool header)
		{
            AppendNewline =true;
            SetLevel(level);
            ShowHeaders = header;
        }
	};


	namespace detail
	{
		struct LineComponent
		{
			bool IsWhitespace = false;
			int Start;
			int End;
			std::string_view * window;
			size_t RealSize;
			LineComponent(int idx)
			{
				Start = idx;
			}
			void Terminate(std::string_view line,int idx)
			{
				window = &line;
				End = idx;
				ComputeSize(line);
			}
			std::string_view Snippet()
			{
				return window->substr(Start,End-Start);
			}
			void ComputeSize(std::string_view line)
			{
				RealSize = 0;
				bool inEscape = false;
				for (int i = Start; i < End; ++i)
				{
					if (line[i] == '\x1b')
					{
						inEscape = true;
						continue;
					}

					if (inEscape)
					{
						if (line[i] == 'm')
						{
							inEscape = false;
						}
					}
					else
					{
						++RealSize;
						if (line[i]=='\t')
						{
							RealSize += 3;
						}
					}
				}
			}
			static std::vector<LineComponent> GetAll(std::string_view line)
			{
				std::vector<detail::LineComponent> bits;
				char notWS = 'a';
				char prevWhitespace = notWS;
				detail::LineComponent current(0);
				current.Start = 0;
				for (int i = 0; i < line.size(); ++i)
				{
					bool isWhitespace = (line[i] == ' ') || (line[i] == '\t');
					if (isWhitespace)
					{
						if (prevWhitespace != line[i]) //not repeated whitespace, or new whitespace
						{
							current.Terminate(line,i);
							bits.push_back(current);
							current = detail::LineComponent(i);
							current.IsWhitespace = true;
						}
						prevWhitespace = line[i];
					}
					else
					{
						if (prevWhitespace != notWS)
						{
							current.Terminate(line,i);
							bits.push_back(current);
							current = detail::LineComponent(i);
						}
						prevWhitespace = notWS;
					}
				}
				current.Terminate(line,line.size());
				bits.push_back(current);
				return bits;
			}
		};
	}


	//! The global ConfigObject used by @ref LOG and LoggerCore to determine what messages are printed, and any associated formatting 
	extern ConfigObject Config;

	//! Used to prevent log-interleaving, and make the LOG (mostly) thread-safe
	extern std::mutex StreamMutex;

	//! A tracker of the number of lines printed since each type of ::LogLevel. Used for LoggerCore::ErasePrevious()
	extern std::vector<size_t> PreviousLines;
}