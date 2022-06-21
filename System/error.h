#pragma once
#include <string>
#include <stdexcept>
#include <vector>
namespace JSL
{


	
	enum ErrorCode {JSLError, SystemError, OverrunError, FailedAssertion,IOError};
	const std::vector<std::string> ErrorNames = {"ERROR", "SYSTEM ERROR", "OVERRUN ERROR", "FAILED ASSERTION","IOError"};
	
	/*!
		Design

	*/
	inline void Error(ErrorCode code, const std::string & message)
	{
		std::string newmessage = "\nJSL " + ErrorNames[code] + ": " + message + "\n";
		throw std::runtime_error(newmessage);
	}

	inline void Error(const std::string & message)
	{
		Error(JSLError,message);
	}

}