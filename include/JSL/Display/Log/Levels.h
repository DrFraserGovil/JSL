#pragma once

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

namespace JSL::Log
{
    //! Convert integers to LogLevels. @param level an integer between 0 and 3 @throws runtime_error if level is out of bounds @returns The corresponding LogLevel 
    LogLevel LogLevelConvert(int level);
   
}

