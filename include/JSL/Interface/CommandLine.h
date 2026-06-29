#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>
namespace JSL::Interface
{
	enum class KeyType
	{
		Flag,
		Value,
		Multivalue,
		String,
	};
	class CommandLine
	{
	  public:
		std::vector<std::string> Commands;
		std::map<std::string, std::string> Arguments;

		CommandLine(int argc, char **argv);
		CommandLine(int argc, char **argv, std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases = {});

	  private:
		std::set<std::string> ResetMultis;
		void ParseConfig(std::string_view configDelim);
		void FlagCheck(const std::string &key, std::vector<std::string> &vals);
		void ValueCheck(const std::string &key, std::vector<std::string> &vals);
		void MultiCheck(const std::string &key, std::vector<std::string> &vals);
		void StringCheck(const std::string &key, std::vector<std::string> &vals);
		std::string CheckKey(std::string &key);
		std::string KeyBuffer;
		std::map<std::string, KeyType> Context;
		std::map<std::string, std::string> Aliases;
	};

} // namespace JSL::Interface
