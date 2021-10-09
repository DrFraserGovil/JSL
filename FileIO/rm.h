#pragma once
#include <dirent.h>
#include <string>
#include "locationExists.h"
#include <stdexcept>
namespace JSL
{
	
	/*!
	 * Calls a system-rm on the provided location, and attempts to remove it.
	 * \param location The target location on which rm is called
	 * \param recursive If true, appends -r to the command, and so removes all subdirectories etc.
	*/
	void inline rm(std::string location, bool recursive)
	{
		bool fileExists = JSL::locationExists(location);
		
		if (fileExists)
		{
			try
			{
				std::string command = "rm ";
				if (recursive)
				{
					command += "-r ";
				}
				command.append(location);
				const char *commandChar = command.c_str(); 
				const int dir_err = system(commandChar);
			}
			catch (const std::exception& e)
			{
				std::string error = "\n\n\nERROR: A problem was encountered trying to remove " + location + ". Error message is as follows\n";
				error = error + std::string(e.what());
				
				throw std::runtime_error(error);
			}
		}
		else
		{
			std::string error =  location + " does not exist. Could not rm this file. Termination will commence";
			throw std::runtime_error(error);
		}
		
	}
	
	
}
