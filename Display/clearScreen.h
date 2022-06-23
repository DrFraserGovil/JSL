#pragma once
#include <iostream>

namespace JSL
{
	//!Deletes the last linebreak character, jumping the cursor up one line
	inline void jumpLineUp()
	{
		printf("\033[F");
	}
	//!Equivalent to calling the "clear" command on the shell, removes all text on the terminal, and returns the cursor to the home position.
	inline void clearScreen()
	{
		// std::cout << "\033[H\033[J" << std::endl;
		printf("\033[H\033[J\033[F");
	}
	//!Removes the last line of text, moving the cursor from its current position, to the beginning of the line. If you want to remove the linebreak which caused that line to exist in the first place, must be followed up with jumpLineUp()
	inline void deleteLine()
	{
		printf("\33[2K\r");
		// std::cout << std::flush;
	}
	
}