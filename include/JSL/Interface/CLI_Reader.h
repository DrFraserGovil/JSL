#pragma once
#include <map>
#include <string>
#include <vector>

namespace JSL::Interface
{
	enum KeyTypes
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
		CommandLine(int argc, char **argv, std::map<std::string, KeyTypes> context);

	  private:
		void FlagCheck(const std::string &key, std::vector<std::string> &vals);
		void ValueCheck(const std::string &key, std::vector<std::string> &vals);
		void MultiCheck(const std::string &key, std::vector<std::string> &vals);
		void StringCheck(const std::string &key, std::vector<std::string> &vals);
	};

} // namespace JSL::Interface
