#pragma once
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string_view>

#include "../Strings/Strings.h"
#include "../utils/jsl_error.h"


#ifdef _WIN32
    #include <io.h>
#else
    #include <unistd.h>
#endif


namespace JSL
{
    //! A compile-time resolve alias for the platform specific isatty command \returns True if the program called with piped input (either via | or <). 
	bool inline PipedInputFound()
	{
        #ifdef JSL_TEST_SPOOF_PIPE
            return true;
        #endif
        #ifdef _WIN32
            return !_isatty(_fileno(stdin));
        #else
            return !isatty(fileno(stdin));
        #endif
	}


    //! A similar macro to forLineIn, but this one iterates through piped input (either from explit pipes "|" or from directed files "<") line by line, exposing the variable PIPE_LINE for further manipulation
    template <typename Func>
    void forLineInPipedInput(Func lineProcessor)
    {
        if (!PipedInputFound())
        {
            internal::FatalError("I/O Error") << "No piped input detected";
        }

        std::string pipe_line;
        while(std::getline(std::cin,pipe_line))
        {
            lineProcessor(pipe_line);
        }
    }

    template <typename Func>
    void forSplitLineInPipedInput(Func lineProcessor,std::string_view delimiter)
    {
        forLineInPipedInput([&](std::string_view line){
            lineProcessor(JSL::split(line,delimiter));
        });
    }

	
	// !Performs the most basic readin --> saves the piped input to a string, and returns it
	std::string inline getPipedInput()
    {
        std::ostringstream out;
        int i = 0;
        forLineInPipedInput([&](std::string_view line){
            if (i > 0)
            {
                out << "\n";
            }
            out << line;
            ++i; 
        });
        return out.str();
    }

}
