#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "../Strings/split.h"


/*!
	Iterates through the file line-by-line (until EOF), saving the current line to ``std::string FILE_LINE``
	* \param macroFileName The target file to search through
	* \param ... A custom block of C++ code which executes on every line of the file. You may use any externally defined variables within this block.
	* \return  Current line is accessible via the variable ``std::string FILE_LINE``. If the file does not exist, throws an ``exit(1)`` command
*/
#define forLineIn(macroFileName, ...)\
{								\
	do 							\
	{							\
		std::ifstream macroFile(macroFileName);	\
		if (!macroFile.is_open())	\
		{							\
			std::cout << "\n\nERROR: Could not find the file '" << macroFileName << ".\n\nPlease provide a valid filepath.\n\n " << std::endl;	\
			exit(1);				\
		}							\
		std::string FILE_LINE;				\
		while (getline(macroFile,FILE_LINE))	\
		{							\
			__VA_ARGS__				\
		}							\
		macroFile.close();			\
	} while(0);						\
}									\


/*!
	Iterates through the file line-by-line (until EOF), saving the current line to ``std::string FILE_LINE``, and then tokenises it using split(), based on the chosen delimiter, saving it to ``std::vector<std::string>> FILE_LINE_VECTOR``
	* \param macroFileName The target file to search through
	* \param token	The delimiter used to break up ``FILE_LINE`` into ``FILE_LINE_VECTOR``
	* \param ... A custom block of C++ code which executes on every line of the file. You may use any externally defined variables within this block.
	* \return  Current line is accessible via the variable ``std::string FILE_LINE`` and ``std::vector<std::string>> FILE_LINE_VECTOR``. If the file does not exist, throws an ``exit(1)`` command
*/
#define forLineVectorIn(macroFileName, token,...)\
{								\
	forLineIn(macroFileName,					\
			std::vector<std::string> FILE_LINE_VECTOR = JSL::split(FILE_LINE,token);	\
			__VA_ARGS__;				\
	);									\
}		
