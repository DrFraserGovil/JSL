#pragma once
#include <dirent.h>
#include <string>
#include "locationExists.h"
#include <stdexcept>
#include "../System/System.h"
namespace JSL
{
	
	/*!
	 * Calls a system-rm on the provided location, and attempts to remove it.
	 * \param location The target location on which rm is called
	 * \param recursive If true, appends -r to the command, and so removes all subdirectories etc.
	*/
	void inline rm(std::string location, bool recursive)
	{
		std::string command = "rm ";
		if (recursive)
		{
			command += "-r ";
		}
		command += location;
		systemCall(command);	
	}
	
	
}
