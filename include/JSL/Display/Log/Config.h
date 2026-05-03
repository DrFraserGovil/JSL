#pragma once
#include <JSL/Display/Log/Levels.h>
#include <JSL/Display/ANSI_Codes.h>
#include <unistd.h> // For isatty()
#include <cstdio>   // For fileno() and stderr
#include <mutex>
namespace JSL::Log
{
    /*!
	\brief A packager for globally accessible variables for the LoggerCore object to refer to. 

	\details A single \c ConfigObject exists, the globally defined \ref Log::Config. Defined by the Initialise_JSL_Log macro call
	*/
	struct ConfigObject
	{
		public:
			//! If true, all Log commands have a newline automatically appended to them. Default: TRUE
			bool AppendNewline;
			
			//! If true, all Log commands are preceeded by 'header info' about the type of log. Headers occur only once per LOG call, linebreaks are automatically aligned. Default: TRUE
			bool ShowHeaders; 
			
			//! Any calls to @ref LOG with a value higher than this are ignored. Default: INFO
			LogLevel Level; 

			//! Automatically detected at runtime-start. True if the output stream is a tty terminal capable of interpreting @ref ANSI commands.
			bool TerminalOutput; 

			bool ForceClear = false;

			Format::Command WarnColour = JSL::Format::Purple;
			Format::Command ErrorColour = JSL::Format::Red;
			Format::Command InfoColour = JSL::Format::White;
			Format::Command DebugColour = JSL::Format::Blue;

			bool DebugBoxing = true;
			size_t DebugLineSize = 80;

			//! Default initialiser. Initialises TerminalOutput, and sets Level=INFO, ShowHeaders=true and AppendNewline=true.
			ConfigObject();
			
			
			//! Convenient interface for setting Level from integer values
			void SetLevel(int level);
			void SetPrompt(std::string_view prompt);
			void ResetPrompt();
			bool IncludePrompt = false;
			std::string Prompt = "";
		private:
        

		/*!
			Used to initialise globally \ref Config, after the command line arguments have been parsed.
			@param level The (as an integer) to be assigned to the Level
			@param header The value to be assigned to ShowHeaders
			@param welcomeFile The file location of the RAMICES 'welcome.dat' file, which prints a cute little message
		*/ 
		void Initialise(int level,bool header);
	};


    namespace Global
    {
        //! @brief The global ConfigObject used by @ref LOG and Core to determine what messages are printed, and any associated formatting 
        extern ConfigObject Config;

        //! Used to prevent log-interleaving, and make the LOG (mostly) thread-safe
        extern std::mutex StreamMutex;
    }
}