#pragma once
#include <cmath> //for size_t

namespace JSL::Terminal
{
	//! A POD holder for the Columns/Rows (measured in characters) in the current terminal
	struct Size
	{
		size_t Columns;
		size_t Rows;
	};

	//! @brief Return the dimensions of the current terminal window (in characters)
	//! @details A linux specific function (for now) that uses ioctl to query the dimensions of STDOUT_FILENO
	//! @returns A Size object containing the dimension information	
	Size GetDimensions();

}