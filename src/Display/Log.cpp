#include <JSL/Display/Log.h>




namespace JSL::Log
{
	//Complex linebreaking algorithm for the lineboxing mode
	std::pair<std::vector<std::string_view>,std::vector<int>> debugBox(std::vector<std::string_view> initial)
	{

		std::vector<std::string_view> out;
		std::vector<int> lineSizes;

		for (auto line : initial)
		{
			auto components = detail::LineComponent::GetAll(line);
			int lineStart = 0;
			int lineSize = components[0].RealSize;
			for (size_t i = 1; i < components.size(); ++i)
			{
				auto & component = components[i];
				size_t proposedLineSize=lineSize + component.RealSize;
				if (proposedLineSize > Config.DebugLineSize)
				{
					int lineEnd = components[i-1].End;
					out.push_back(line.substr(lineStart,lineEnd-lineStart));
					lineSizes.push_back(lineSize);

					if (component.IsWhitespace)
					{
						if (i < components.size() -1)
						{
							i+=1;
							component = components[i];
							lineStart = component.Start;
							lineSize = component.RealSize;
						}
						else
						{
							lineStart = -1; //signal there's no awaiting 
						}
					}
					else
					{
						lineStart = component.Start;
						lineSize = component.RealSize;
					}
				}
				else
				{
					lineSize = proposedLineSize;
				}
			}
			if (lineSize > -1)
			{
				out.push_back(line.substr(lineStart,std::string_view::npos));
				lineSizes.push_back(lineSize);
			}
		}
		return {out,lineSizes};
	}

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
		if (Level == DEBUG && Config.DebugBoxing)
		{
			auto p = std::filesystem::path(callingFile).filename();
			LineSuffix = "|";
			FirstLineSuffix = "| (" + p.string() + ": "+ std::to_string(callingLine) + ") ";
			// int remain = std::max((size_t)0,Config.DebugLineIndent - Insert.size());
			// if (remain > 0) Insert += std::string(remain,' ');
		}
	}

	Core::~Core()
	{
		if (StreamActive) //only add the output to stream if "<<" was actually called
		{
			EndMessage();
		}
	}

	Core & Core::operator<<(Format::Command format)
	{
		if (Config.TerminalOutput)
		{
			CurrentFormat.Add(format);
			Buffer << CurrentFormat;
		}
		return *this;
	}

	Core &Core::operator<<(Format::FormatGroup group)
	{
		if (Config.TerminalOutput)
		{
			CurrentFormat.Add(group);
			Buffer << CurrentFormat;
		}
		return *this;
	}

	void Core::Header()
	{
		std::string_view label;
		Format::Command fmt;
		switch(Level) {
			case DEBUG: fmt = Config.DebugColour;label = "[DEBUG] "; break;
			case INFO: fmt=Config.InfoColour;label = "[INFO]  "; break;
			case WARN: fmt=Config.WarnColour;label = "[WARN]  "; break;
			case ERROR: fmt=Config.ErrorColour;label = "[ERROR] "; break;
			default: throw std::runtime_error("Invalid logger argument");
		} 
		if (Config.ForceClear)
		{
			BufferPreamble << JSL::Terminal::ClearLine;
		}
		if (Config.TerminalOutput)
		{
			BufferPreamble << fmt;
		}
		if (Config.ShowHeaders)
		{
			BufferPreamble << label;
		}
	}

	void Core::EndMessage()
	{
		//now format the data so that linebreaks are suitably indented
		std::string linebreak = "\n";
		std::vector<int> lineSizes;
		if (Config.ShowHeaders)
		{
			linebreak += "\t";
		}
		
		std::vector<std::string_view> message = split_view(Buffer.view(),"\n");
		const bool needBoxing = (Level == DEBUG && Config.DebugBoxing);
		if (needBoxing)
		{
			auto [out,counts] = debugBox(message);
			std::swap(out,message);
			std::swap(lineSizes,counts);
		}
		std::string buffer = "";

		//iterate across all lines in the entry 
		{
			std::unique_lock<std::mutex> lock(Log::StreamMutex); //lock the stream to prevent interleaving

			auto getPadding = +[](int i,std::vector<int> & sizes)->std::string{return "";};
			if (needBoxing)
			{
				getPadding = +[](int i,std::vector<int> & sizes)->std::string{
					int amount = Config.DebugLineSize - sizes[i];
					std::ostringstream out;
					if (amount > 0)
					{
						out << std::string(amount,' ');
					}
					out << "\033[0m" << Config.DebugColour;
					return out.str();  
				};
			}
			
			//the first line automatically includes the correct indentation -- the header accounts for that
			std::cout << BufferPreamble.view()  << message[0]  << getPadding(0,lineSizes) << FirstLineSuffix; 

			//subequent lines need to indent (or not) based on the presence of the header.
			for (size_t i = 1; i < message.size(); ++i)
			{
				std::cout << linebreak << CurrentFormat << message[i] << getPadding(i,lineSizes) << LineSuffix;
			}
			if (Config.AppendNewline)
			{
				std::cout << "\n"; 
			}
			if (Config.TerminalOutput)
			{
				std::cout << Format::ResetAll;
			}

			//save the data to the 'erase' memory banks
			auto nlines = message.size();
			Log::PreviousLines[Level] =0;
			for (int i = 0; i < LogLevel::MAXLEVEL; ++i)
			{ 
				Log::PreviousLines[i] += nlines; //do this inside the mutex so line ordering is correct
			}
		}
	}
}