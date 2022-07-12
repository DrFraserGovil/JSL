#pragma once
#include "locationExists.h"
#include <fstream>
#include <limits>
#include "../System/assert.h"
namespace JSL
{
	int LineCount(std::string file)
	{
		bool exists = JSL::locationExists(file);
		JSL::Assert("Can only count lines in files which exist",exists);
		std::fstream in(file);
		int lines = 0;
		char endline_char = '\n';
		while (in.ignore(std::numeric_limits<std::streamsize>::max(), in.widen(endline_char)))
		{
			++lines;
		}
		return lines;
	}
}