#pragma once
#include <string>
#include <sys/stat.h>


namespace JSL
{
	
	/*!
	 * Checks for the existence of the provided file location -- works on both files and directories.
	 * \param filename the name of the file or directory to be checked
	 * \returns true if location exists (and is accessible), false if not
	*/ 
	bool locationExists(const std::string & filename)
	{
		struct stat buffer;   
		return (stat (filename.c_str(), &buffer) == 0); 
	}
}
