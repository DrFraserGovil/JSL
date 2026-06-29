#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>
namespace JSL::internal::Parameter
{
	/*
			TODO: Think about the best way to restructure this system; the current
	   Parameter-first way is a bit of an arrtefact INFO: Do we want to establish a
	   'parser' and a 'modifier' struct that can be attached either to a value; so
	   rather than Parameter<T> x(default,"trigger",argc,argv), you might declare T
	   x = Interface::Parse(default,"trigger",argc,argv) -- emphasising that the
	   parsing is transient?

			HACK: This method is only exposed to the API becuase it is used in the
	   all CLI parsers: it should be sequestered away
	*/
	inline std::string_view Normalize(std::string_view s)
	{
		const size_t start = s.find_first_not_of('-');
		// If it's all dashes, return an empty view; otherwise, strip them.
		return (start == std::string_view::npos) ? "" : s.substr(start);
	}

	class CommandLineReader
	{
	  public:
		static CommandLineReader &Get()
		{
			static CommandLineReader instance;
			return instance;
		}
		CommandLineReader() = default;
		CommandLineReader(int argc, char **argv);
		void Parse(int argc, char **argv);
		void Configure(std::filesystem::path configFile,
			std::string_view configDelimiter);
		void Reset();
		bool IsConfigured();
		bool Contains(std::string_view option) const;
		std::string_view GetOption(std::string_view key) const;
		std::vector<std::string> Commands;
		std::map<std::string, std::string> Options;
		bool Configured = false;
	};

} // namespace JSL::internal::Parameter
