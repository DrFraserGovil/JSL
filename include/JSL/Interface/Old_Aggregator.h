#pragma once
#include <JSL/Interface/NestedAggregator.h>

namespace JSL::Parameter
{
	class Aggregator : public NestedAggregator
	{
	  public:
		Aggregator(std::string_view name);
		void Parse(int argc, char **argv);

		Aggregator();

		std::vector<std::string> InvokedCommands();
		std::pair<std::set<std::string>, std::set<std::string>> ParseCommands();
		void HelpMenu();

	  protected:
		static std::map<std::string, std::string> &RegisteredCommands();
		std::string &GetDefaultCommand();
		void AddCommand(std::string name, std::string result);
		void DefaultCommand(std::string name, std::string result);
		std::string Name = "Root";
		bool Help;
		Setting<bool> HelpToggle;
	};
} // namespace JSL::Parameter
