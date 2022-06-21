#pragma once
#include <iostream>

namespace JSL
{
	inline void jumpLineUp()
	{
		printf("\033[F");
	}
	inline void clearScreen()
	{
		// std::cout << "\033[H\033[J" << std::endl;
		printf("\033[H\033[J");
		jumpLineUp();
	}
	inline void deleteLine()
	{
		printf("\33[2K\r");
		// std::cout << std::flush;
	}
	
}