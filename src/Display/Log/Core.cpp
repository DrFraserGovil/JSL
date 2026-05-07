#include <JSL/Display/Log.h>
#include <JSL/Strings.h>

#include "LineBreaker.h"
#include <filesystem>


namespace JSL::Log
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
		if (Global::Config().Level == DEBUG || Level <= WARN)
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
			case DEBUG: fmt = Global::Config().DebugColour;label = "[DEBUG] "; break;
			case INFO: fmt=Global::Config().InfoColour;label = "[INFO]  "; break;
			case WARN: fmt=Global::Config().WarnColour;label = "[WARN]  "; break;
			case ERROR: fmt=Global::Config().ErrorColour;label = "[ERROR] "; break;
			default: throw std::runtime_error("Invalid logger argument");
		} 
		if (Global::Config().ForceClear)
		{
			BufferPreamble << JSL::Terminal::ClearLine;
		}
		if (Global::Config().TerminalOutput && !ManualFormat)
		{
			BufferPreamble << fmt;
		}
		if (Global::Config().ShowHeaders)
		{
			BufferPreamble << label;
		}
	}

	void Core::EndMessage()
	{
		//now format the data so that linebreaks are suitably indented
		std::string linebreak = "\n";
		if (Global::Config().ShowHeaders)
		{
			linebreak += std::string(8,' ');
		}
		
		std::vector<std::string_view> manualSplits = String::split_view(Buffer.view(),"\n");
		std::vector<std::string_view> message;

		for (auto line : manualSplits)
		{
			auto folded = JSL::String::foldToWidth(line,Global::Config().LineSize);
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
		if (Global::Config().TerminalOutput)
		{
			std::cout << Format::ResetAll;
		}

		if (Global::Config().AppendNewline)
		{
			std::cout << "\n"; 
		}

		//lock passes out of scope, and hence unlocks
	}
}