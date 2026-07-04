#pragma once
#include <string_view>
#include <functional>

namespace JSL::IO::Pipe
{
    //! A compile-time resolve alias for the platform specific isatty command \returns True if the program called with piped input (either via | or <). 
	bool IsPiped();


    //! A similar function to forLineIn, but this one iterates through piped input (either from explit pipes "|" or from directed files "<") line by line, calling the provided function for each line
    //! This code acts sequentially, and will wait for each complete line to be passe dthrough the pipe, until it is shut down
    void forLineIn(std::function<void(std::string_view)>lineProcessor);

    //! @brief As with forLineIn, but additionally splits the line based on a delimiter
    //! This code acts sequentially, and will wait for each complete line to be passe dthrough the pipe, until it is shut down
    void forSplitLineIn(std::function<void(std::vector<std::string_view>)>lineProcessor,std::string_view delimiter);

	
	//! @brief Performs the most basic readin: saves the piped input to a string
    //! @returns A string representing all input given to the pipe before it shuts
    //! @warning The code will hang here until the pipe closes. For long-running processes this may cause errors.
	std::string save();

}
