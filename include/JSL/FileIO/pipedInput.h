#pragma once
#include <string_view>
#include <functional>

namespace JSL::Input
{
    //! A compile-time resolve alias for the platform specific isatty command \returns True if the program called with piped input (either via | or <). 
	bool IsPiped();


    //! A similar function to forLineIn, but this one iterates through piped input (either from explit pipes "|" or from directed files "<") line by line, calling the provided function for each line
    void forLineInPipe(std::function<void(std::string_view)>lineProcessor);

    void forSplitLineInPipe(std::function<void(std::vector<std::string_view>)>lineProcessor,std::string_view delimiter);

	
	// !Performs the most basic readin --> saves the piped input to a string, and returns it
	std::string savePipe();

}
