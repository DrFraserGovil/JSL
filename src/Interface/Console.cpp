#include <JSL/Interface/Console.h>

#include <JSL/Log.h>
#include <JSL/Strings/Cases.h>
#include <JSL/Strings/Split.h>
#include <JSL/Strings/Stitch.h>
namespace JSL::Interface
{

	std::string cmd(std::string input, JSL::Display::Format col)
	{
		return col + input + JSL::Display::ResetAll();
	}
	void ConsoleHelper::InitialPrompt()
	{
		if (!ShowIntro) return;
		LOG(INFO) << cmd(ShowString + " <key>", CmdColor) << " to see the current setting value";
		LOG(INFO) << cmd(SetString + " <key> <val>", CmdColor) << " to update the settings of the archive";
		LOG(INFO) << cmd(HelpString, CmdColor) << " to see the available keys";
		LOG(INFO) << cmd(ExitString, CmdColor) << " to leave the interface";
	}
	void ConsoleHelper::Warn(std::string_view string)
	{
		LOG(WARN) << string;
	}
	void ConsoleHelper::Print(std::string_view string)
	{
		// a bit hacky, but prints using the current INFO styling, but bypasses the conditional check, so prints whenever
		JSL::Log::internal::Core(INFO, __LINE__, __func__, __FILE__) << string;
	}

	void ConsoleHelper::Unformat()
	{
		std::cout << JSL::Display::DefaultColour();
	}
	ConsoleHelper::CmdTokens ConsoleHelper::Tokenise(std::string command, ContextMap context)
	{
		auto tokens = JSL::String::split(command, " ");

		// remove leading spaces
		while (!tokens.empty() && tokens[0].empty())
		{
			tokens.erase(tokens.begin());
		}
		if (tokens.empty())
		{
			return {false, {}, {}};
		}
		if (caseInsensitive)
		{
			JSL::String::toLower(tokens[0]);
		}
		if (tokens[0] == ExitString)
		{
			return {true, {}, {}};
		}
		if (tokens[0] == HelpString)
		{
			return {false, HelpString, {}};
		}

		bool validCmd = false;
		std::string target;
		std::string args;
		if (tokens[0] == SetString)
		{
			if (tokens.size() < 3)
			{
				LOG(WARN) << "Use the syntax " << SetString << " <key> <value>";
				return {false, {}, {}};
			}
			validCmd = true;
			target = tokens[1];
			args = JSL::String::stitch(tokens, 1, tokens.size(), " ");
		}
		if (tokens[0] == ShowString)
		{
			if (tokens.size() != 2)
			{
				LOG(WARN) << "Use the syntax " << ShowString << " <key>";
				return {false, {}, {}};
			}
			validCmd = true;
			target = tokens[1];
			args = tokens[1];
		}

		if (validCmd)
		{
			if (!context.Contains(target))
			{
				LOG(WARN) << "Key '" << target << "' not recognised";
				return {false, {}, {}};
			}
			return {false, tokens[0], args};
		}
		else
		{
			if (!UnrecognisedCommand.empty())
			{
				LOG(WARN) << UnrecognisedCommand;
			}
			return {false, {}, {}};
		}
	}

	void ConsoleHelper::LoopPrompt()
	{
		std::cout << ">>  " << CmdColor << std::flush;
	}

} // namespace JSL::Interface
