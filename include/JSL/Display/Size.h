#pragma once
#include <cmath> //for size_t

namespace JSL::Terminal
{
	struct Size
	{
		size_t Columns;
		size_t Rows;
	};

	Size GetDimensions();

}