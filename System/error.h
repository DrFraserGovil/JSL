#pragma once
#include <string>
#include <stdexcept>
#include <vector>
namespace JSL
{


	//!A list of recognised JSL errors
	enum ErrorCode {JSLError, //!< Global error term
		SystemError, //!< Used when something really went bad, like a failed JSL::systemCall
		OverrunError, //!< Vector Access overruns, bad memory requests
		FailedAssertion, //!< As the name implies, failed assertions
		IOError //!< Errors induced by file
	};
	const std::vector<std::string> ErrorNames = {"ERROR", "SYSTEM ERROR", "OVERRUN ERROR", "FAILED ASSERTION","IOError"};
	
	/*!
		An easy way to package throwing errors using the throw command. \param code One of JSL::ErrorCode to help identify the cause \param message The output message
	*/
	inline void Error(ErrorCode code, const std::string & message)
	{
		std::string newmessage = "\nJSL " + ErrorNames[code] + ": " + message + "\n";
		throw std::runtime_error(newmessage);
	}

	//! The default version of Error(ErrorCode code, const std::string & message), throwing JSL::JSLError as the identifier code. \param message The error message passed to the throw command. 
	inline void Error(const std::string & message)
	{
		Error(JSLError,message);
	}

}