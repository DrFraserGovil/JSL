#pragma once
#include <string>
#include <stdexcept>

namespace JSL
{


	
	enum ErrorCode {JSLError, SystemError, OverrunError, FailedAssertion,IOError};
	std::vector<std::string> ErrorNames = {"ERROR", "SYSTEM ERROR", "OVERRUN ERROR", "FAILED ASSERTION","IOError"};
	
	/*!
		Design

	*/
	void Error(ErrorCode code, const std::string & message)
	{
		std::string newmessage = "\nJSL " + ErrorNames[code] + ": " + message + "\n";
		throw std::runtime_error(newmessage);
	}

	void Error(const std::string & message)
	{
		Error(JSLError,message);
	}

}