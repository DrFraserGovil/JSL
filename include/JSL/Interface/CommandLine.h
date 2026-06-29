#pragma once
#include "KeyMapper.h"
#include <map>
#include <string>
#include <vector>
namespace JSL::Interface
{
	class CommandLine
	{
	  public:
		std::vector<std::string> Commands;
		std::map<std::string, std::string> Arguments;

		CommandLine(int argc, char **argv);
		CommandLine(int argc, char **argv, std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases = {});

	  protected:
		void FlagCheck(const std::string &key, std::vector<std::string> &vals);
		void ValueCheck(const std::string &key, std::vector<std::string> &vals);
		void MultiCheck(const std::string &key, std::vector<std::string> &vals);
		void StringCheck(const std::string &key, std::vector<std::string> &vals);
		std::string KeyBuffer;
		KeyMapper Keys;
	};

	class ConfigurableCommandLine : public CommandLine
	{
	  public:
		ConfigurableCommandLine(int argc, char **argv, std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases = {});
	};

} // namespace JSL::Interface
