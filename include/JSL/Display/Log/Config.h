#pragma once
#include <JSL/Display/Log/Levels.h>
#include <JSL/Display/ANSI_Codes.h>
#include <unistd.h> // For isatty()
#include <cstdio>   // For fileno() and stderr
#include <mutex>
namespace JSL::Log
{
	class Core; //forward declaration for friend
    /*!
	\brief A packager for globally accessible variables for the LoggerCore object to refer to. 

	\details Only one \c ConfigObject should exist: the one accessed through the globally defined singleton \ref Log::Global::Config. 
	*/
	class ConfigObject
	{
		public:
			
			//! Default initialiser. Initialises TerminalOutput, and sets Level=INFO, ShowHeaders=true and AppendNewline=true.
			ConfigObject();
			
			/*!
				Used to initialise globally \ref Config, after the command line arguments have been parsed.
				@param level The (as an integer) to be assigned to the Level
				@param header The value to be assigned to ShowHeaders
				@param welcomeFile The file location of the RAMICES 'welcome.dat' file, which prints a cute little message
			*/ 
			void Initialise(int level,bool header);

			//! Convenient interface for setting Level from integer values
			void SetLevel(int level);


			//! 
			void AlignSize(size_t debugReserve = 30);
		
			//! Any calls to @ref LOG with a value higher than this are ignored. Default: INFO
			LogLevel Level; 

			friend class JSL::Log::Core;
		private:

			//! If true, all Log commands have a newline automatically appended to them. Default: TRUE
			bool AppendNewline;
			
			//! If true, all Log commands are preceeded by 'header info' about the type of log. Headers occur only once per LOG call, linebreaks are automatically aligned. Default: TRUE
			bool ShowHeaders; 
			
			
			//! Automatically detected at runtime-start. True if the output stream is a tty terminal capable of interpreting @ref ANSI commands.
			bool TerminalOutput; 

			bool ForceClear = false;

			Format::Command WarnColour = JSL::Format::Purple;
			Format::Command ErrorColour = JSL::Format::Red;
			Format::Command InfoColour = JSL::Format::White;
			Format::Command DebugColour = JSL::Format::Blue;

			size_t LineSize = 80;


			
	};


    namespace Global
    {
        //! @brief A singleton for accessing the global ConfigObject used by @ref LOG and Core to determine what messages are printed, and any associated formatting 
		ConfigObject & Config();


    }
}