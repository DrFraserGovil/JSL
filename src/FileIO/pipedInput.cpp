#include <JSL/FileIO/pipedInput.h>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <string_view>

#include <JSL/Strings/Manipulate.h>
#include <JSL/internal/error.h>


#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
#else
    #include <unistd.h>
#endif


namespace JSL::IO::Pipe
{
    //! A compile-time resolve alias for the platform specific isatty command \returns True if the program called with piped input (either via | or <). 
	bool IsPiped()
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

    // !Performs the most basic readin --> saves the piped input to a string, and returns it
	std::string save()
    {
        std::ostringstream out;
        int i = 0;
        forLineIn([&](std::string_view line){
            if (i > 0)
            {
                out << "\n";
            }
            out << line;
            ++i; 
        });
        return out.str();
    }


   void forLineIn(std::function<void(std::string_view)>lineProcessor)
    {
        if (!IsPiped())
        {
            internal::FatalError("I/O Error", JSL_LOCATION)  << "No piped input detected";
        }

        std::string pipe_line;
        while(std::getline(std::cin,pipe_line))
        {
            lineProcessor(pipe_line);
        }
    }


   void forSplitLineIn(std::function<void(std::vector<std::string_view>)>lineProcessor,std::string_view delimiter)
    {
        forLineIn([&](std::string_view line){
            lineProcessor(JSL::String::split_view(line,delimiter));
        });
    }



}
