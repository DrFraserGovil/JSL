#include <JSL/internal/error.h>

#include "JSL/Display/Log.h"

#include <stdexcept>
#include <filesystem>
#include <algorithm>

namespace JSL::internal
{
	namespace fs = std::filesystem;
	std::string truncatedFilePath(const std::string & fullPath,std::string_view target)
	{
		std::filesystem::path p(fullPath);
		
		auto it = std::find(p.begin(), p.end(), target);

		if (it != p.end()) 
		{
			fs::path result;
			for (; it != p.end(); ++it)
			{
				result = result/ *it;
			}
			return result;
		}

		return fullPath; // Or return empty if not found
	}

	FatalError::FatalError(std::string msg,int callingLine,const std::string & callingFunction,std::string callingFile) : Summary(msg)
	{
		auto shortFile = truncatedFilePath(callingFile,"JSL");
		line = callingLine;
		function = callingFunction;
		file = shortFile;
		// JSL::Log::Core(ERROR,callingLine,callingFunction,shortFile);  << msg;
	}

	FatalError::~FatalError() noexcept (false)
	{
		JSL::Log::Core(ERROR,line,function,file) << Summary << "\n" << Buffer.str() <<"\n";
		throw std::runtime_error(Summary);
	}
		
}