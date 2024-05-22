#pragma once
#include <unistd.h>
#include <iostream>
#include "error.h"
//! A similar macro to forLineIn, but this one iterates through piped input (either from explit pipes "|" or from directed files "<") line by line, exposing the variable PIPE_LINE for further manipulation
#define forLineInPipedInput( ... )\
{								\
	if (!JSL::PipedInputFound()){		\
		Error(JSL::IOError,"Cannot Read Piped input from standard input channel");}	\
	do 							\
	{							\
		std::string PIPE_LINE;				\
		while (getline(std::cin,PIPE_LINE))	\
		{							\
			__VA_ARGS__				\
		}							\
	} while(0);						\
}									\

//! A similar macro to forLineIn, but this one iterates through piped input (either from explit pipes "|" or from directed files "<") line by line, exposing the variable PIPE_LINE for further manipulation
#define forLineVectorInPipedInput(token,...)\
{								\
	forLineInPipedInput(					\
			std::vector<std::string> PIPE_LINE_VECTOR = JSL::split(PIPE_LINE,token);	\
			__VA_ARGS__;				\
	)									\
}		
namespace JSL
{
	//! An alias for an isatty command, given a easier to remember name \returns True if the program called with piped input (either via | or <). Does not work on Windows.
	bool inline PipedInputFound()
	{
		return !isatty(fileno(stdin));
	}

	//!Performs the most basic readin --> saves the piped input to a string, and returns it
	std::string inline readPipedInputString()
	{
		std::string s;
		int i = 0;
		forLineInPipedInput(
			if (i > 0)
			{
				s+="\n"; //prevents including the final terminal line break associated with the EOF
			}
			s += PIPE_LINE;
			++i;
		)
		return s;
	}
}
