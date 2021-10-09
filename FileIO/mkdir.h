#pragma once
#include <dirent.h>
#include <string>
#include "locationExists.h"

namespace JSL
{
	
	//!A wrapper for the return type of mkdirSafely()
	struct mkdirReturn
	{
		//!True if the directory already existed, or was succesfully created.
		bool Successful;
		//!Contains the logging messages from the function
		std::string Message;
	};
	
	
	/*!
		  Checks the status of the target directory, if it does not exist, attempts to create it. Works wherever the `mkdir` command is installed.   
		  \param directory Path (relative or absolute) to the desired directory
		  * \return A mkdirReturn object detailing the success + associated messages for the request
		*/
	mkdirReturn inline mkdir(std::string directory)
	{
		
		mkdirReturn output;
		output.Successful = true;
		output.Message = "";
		
		
		
		if (JSL::locationExists(directory))
		{
			output.Message += "\tDirectory '" + directory + "' already exists. Request to mkdir ignored.\n";
		}
		else
		{
			output.Message += "Directory '" + directory +"'  does not exist. Constructing directory...\n";
			try
			{
				std::string command = "mkdir -p ";
				command.append(directory);
				const char *commandChar = command.c_str(); 
				const int dir_err = system(commandChar);
			}
			catch (const std::exception& e)
			{
				std::string error = "\n\n\nERROR: A problem was encountered trying to create directory. Error message is as follows\n";
				error = error + std::string(e.what());
				output.Message += error;
				output.Successful = false;
			}
		}
		
		return output;
	}
}
