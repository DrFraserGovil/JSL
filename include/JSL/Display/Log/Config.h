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

	\details The Constructor is private, so only one Log::Config can exist: the one accessed through the Global() member (a Meyer's Singleton). 
	*/
	class Config
	{
		public:
			//! @brief A singleton for accessing the global ConfigObject used by @ref LOG and Core to determine what messages are printed, and any associated formatting 
			static Config & Global();
			
	
			//! @brief Convenient interface for setting Level from integer values
			//! @details Equivalent to \c Level=Log::MakeLevel(), but slightly less verbose.
			//! @param level an integer between 0 and 3
			//! @throws runtime_error if level is out of bounds
			void SetLevel(int level);

			/*!
				@brief Manually disables terminal-specific code generation
				@details Text colouration, formatting and cursor movement commands do not translate well into non-terminal environments (such as piped into a file). The module uses Terminal::IsANSICapable() to auto-detect this and disable if necessary; this provides a sledgehammer method to remove format commands. 
			*/
			void DisableTerminal();

			/*! 
				@brief Extracts the current size of the output terminal (if one exists), and computes the line-folding size.
				@details This is called once at startup, and then can either be called manually, or set to re-detect with every log call.
				@param debugReserve The number of characters to reserve at the right margin for debugging information about the logs
			*/
			void AlignSize(size_t debugReserve = 30);
		
			/// @name Configuration Variables
			/// @{

				/*!
					@brief The global filter level for the logs. Any calls to @ref LOG with a value higher than this are ignored. Default: INFO
				*/
				LogLevel Level = LogLevel::INFO; 

				//! @brief If true, all Log commands have a newline automatically appended to them. Default: TRUE
				bool AppendNewline = true;

				//! @brief If true, all Log commands are preceeded by 'header info' about the type of log. Headers occur only once per LOG call, linebreaks are automatically aligned. Default: TRUE
				bool ShowHeaders = true; 

				//! @brief If true, LOG clears the current terminal line and resets the cursor position. Useful to prevent interleaving of superfluous cout messaging. Default: FALSE
				bool ForceClear = false;

			/// @}

			//! If Formatting active, the default colour assigned to LOG(ERROR) messages
			Format::Command ErrorColour = JSL::Format::Red;

			//! If Formatting active, the default colour assigned to LOG(WARN) messages
			Format::Command WarnColour = JSL::Format::Purple;
			
			//! If Formatting active, the default colour assigned to LOG(INFO) messages
			Format::Command InfoColour = JSL::Format::White;

			//! If Formatting active, the default colour assigned to LOG(DEBUG) messages
			Format::Command DebugColour = JSL::Format::Blue;


			//! The Core class needs to be able to reach inside the private members to access them. 
			friend class JSL::Log::Core;
		private:
			/*! 
				@brief Initialises default values, calls Terminal::IsANSICapable() and AlignSize() to get the state of the terminal
				@details Set to private to enforce singleton uniqueness; only the singleton function should call this
			*/
			Config();
			
			//! Automatically detected at runtime-start. True if the output stream is a tty terminal capable of interpreting @ref ANSI commands.
			bool TerminalOutput; 

			

			

			//! The stored linesize cached by AlignSize
			size_t LineSize = 80;

			//! @brief Deleted to enforce Singleton uniqueness
			Config(const Config& other) = delete;
			//! @brief Deleted to enforce Singleton uniqueness
			Config& operator=(const Config& other) = delete;
			//! @brief Deleted to enforce Singleton uniqueness
			Config(Config&& other) = delete;
			//! @brief Deleted to enforce Singleton uniqueness
			Config& operator=(Config&& other) = delete;
			
	};

}