#pragma once
#include "error.h"


namespace JSL
{


	inline void systemCall(const std::string & command)
	{
		int errorStatus = std::system(command.c_str());

		if (errorStatus != 0)
		{
			Error(SystemError,"The system call \'" + command + "\' returned error " + std::to_string(errorStatus));
		}
	}

}