#include <JSL/Display/Log.h>
#include <JSL/Strings.h>

#include "LineBreaker.h"
#include <filesystem>


namespace JSL::Log::internal
{
	
	Core::Core(LogLevel level,int callingLine,const std::string & callingFunction,std::string callingFile)
	{
		StreamActive = false;
		Level = level;
		Insert = "";
		if (Level == ERROR)
		{
			Insert = "Line " + std::to_string(callingLine) + " of " + callingFile + " in function " + callingFunction ;
			Insert += "\n";
		}
		if (Config::Global().Level == DEBUG || Level <= WARN)
		{
			auto p = std::filesystem::path(callingFile).filename();
			LineSuffix = "|";
			FirstLineSuffix = "| (" + p.string() + ": "+ std::to_string(callingLine) + ") ";
		}
	}

	Core::~Core()
	{
		if (StreamActive) //only add the output to stream if "<<" was actually called
		{
			EndMessage();
		}
	}

	void Core::Header()
	{
		std::string_view label;
		Format::Command fmt;
		switch(Level) {
			case DEBUG: fmt = Config::Global().DebugColour;label = "[DEBUG] "; break;
			case INFO: fmt=Config::Global().InfoColour;label = "[INFO]  "; break;
			case WARN: fmt=Config::Global().WarnColour;label = "[WARN]  "; break;
			case ERROR: fmt=Config::Global().ErrorColour;label = "[ERROR] "; break;
			default: throw std::runtime_error("Invalid logger argument");
		} 
		if (Config::Global().ForceClear)
		{
			BufferPreamble << JSL::Terminal::ClearLine;
		}
		if (Config::Global().TerminalOutput && !ManualFormat)
		{
			BufferPreamble << fmt;
		}
		if (Config::Global().ShowHeaders)
		{
			BufferPreamble << label;
		}
	}

	void Core::EndMessage()
	{
		//now format the data so that linebreaks are suitably indented
		std::string linebreak = "\n";
		if (Config::Global().ShowHeaders)
		{
			linebreak += std::string(8,' ');
		}
		
		std::vector<std::string_view> manualSplits = String::split_view(Buffer.view(),"\n");
		std::vector<std::string> message; //have to be strings as the folding inserts spaces

		for (auto line : manualSplits)
		{
			auto folded = JSL::String::wrapToWidth(line,Config::Global().LineSize);
			for (auto subline : folded)
			{
				message.push_back(subline);
			}
		}

		//lock the stream to prevent interleaving
		std::lock_guard<std::mutex> lock(StreamMutex); 
		
		//the first line automatically includes the correct indentation -- the header accounts for that
		std::cout << BufferPreamble.view()  << message[0]   << FirstLineSuffix; 

		//subequent lines need to indent (or not) based on the presence of the header.
		for (size_t i = 1; i < message.size(); ++i)
		{
			std::cout << linebreak << CurrentFormat << message[i] << LineSuffix;
		}

		//then terminate the message
		if (Config::Global().TerminalOutput)
		{
			std::cout << Format::ResetAll();
		}

		if (Config::Global().AppendNewline)
		{
			std::cout << "\n"; 
		}

		//lock passes out of scope, and hence unlocks
	}
}