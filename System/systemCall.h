#pragma once
#include "error.h"


namespace JSL
{

	//! Uses C++'s std::system to run a command given by the inpt string, and checks for errors. \param command The command to be executed by the system's default command processor (/bin/bash, cmd.exe etc)
	inline void systemCall(const std::string & command)
	{
		int errorStatus = std::system(command.c_str());

		if (errorStatus != 0)
		{
			Error(SystemError,"The system call \'" + command + "\' returned error " + std::to_string(errorStatus));
		}
	}

}