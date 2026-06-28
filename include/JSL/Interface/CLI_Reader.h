#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <vector>
namespace JSL::internal::Parameter
{
	std::string_view Normalize(std::string_view s);

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
		void Configure(std::filesystem::path configFile, std::string_view configDelimiter);
		void Reset();
		bool IsConfigured();
		bool Contains(std::string_view option) const;
		std::string_view GetOption(std::string_view key) const;
		std::vector<std::string> Commands;
		std::map<std::string, std::string> Options;
		bool Configured = false;
	};
} // namespace JSL::internal::Parameter
