#include <JSL/Interface/Console.h>

#include <JSL/Log.h>
#include <JSL/Strings/Cases.h>
#include <JSL/Strings/Split.h>
#include <JSL/Strings/Stitch.h>
#include <JSL/Vectors/Search.h>
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
		LOG(INFO) << cmd(AddString + "/" + RemoveString + " <key> <val>", CmdColor) << " to add/remove <val> to a vector-type option";
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
		return internalTokenise(tokens, context, false);
	}
	std::pair<std::string, std::string> ConsoleHelper::VectorExtract(std::string input)
	{

		auto t = JSL::String::split(input, "////");
		if (t.size() != 2)
		{
			LOG(WARN) << "Cannot parse " << input << " as a valid key////arg split";
			return {{}, {}};
		}
		return {t[0], t[1]};
	}
	std::string ConsoleHelper::ConstructReducedVector(std::string original, std::string element)
	{

		auto oldToks = JSL::String::split(original, ", ");
		auto loc = JSL::Vector::find(oldToks, element);
		if (loc.Found)
		{
			oldToks.erase(oldToks.begin() + loc.Index);
			return "[" + JSL::String::stitch(oldToks, ", ") + "]";
		}
		else
		{
			LOG(WARN) << element << " is not in [" << original << "], it cannot be removed";
			return "";
		}
	}

	ConsoleHelper::CmdTokens ConsoleHelper::internalTokenise(std::vector<std::string> tokens, ContextMap &context, bool hasInverted)
	{
		std::string cmd = caseInsensitive ? JSL::String::getLower(tokens[0]) : tokens[0];

		if (cmd == ExitString)
		{
			return {true, {}, {}};
		}
		if (cmd == HelpString)
		{
			return {false, HelpString, {}};
		}

		bool validCmd = false;
		std::string target;
		std::string args;
		if (cmd == SetString)
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
		if (cmd == ShowString)
		{
			if (tokens.size() != 2)
			{
				if (tokens.size() == 1)
				{
					return {false, {HelpString}, {}};
				}
				LOG(WARN) << "Use the syntax " << ShowString << " <key>";
				return {false, {}, {}};
			}
			validCmd = true;
			target = tokens[1];
			args = tokens[1];
		}
		if (cmd == AddString || cmd == RemoveString)
		{
			if (tokens.size() < 3)
			{
				LOG(WARN) << "Use the syntax " << tokens[0] << " <key> <value>";
			}

			target = tokens[1];
			validCmd = true;
			args = target + "////" + JSL::String::stitch(tokens, 2, tokens.size(), " ");
			if (context.Contains(target))
			{
				auto [_, type] = context.GetCanonical(target);
				if (type != KeyType::Multivalue)
				{
					LOG(WARN) << "Cannot use vector commands on " << target << ": not a vector type";
					return {false, {}, {}};
				}
			}
		}

		if (validCmd)
		{
			if (!context.Contains(target))
			{
				LOG(WARN) << "Key '" << target << "' not recognised";
				return {false, {}, {}};
			}
			return {false, cmd, args};
		}
		else
		{
			if (hasInverted || tokens.size() < 2)
			{

				if (!UnrecognisedCommand.empty())
				{
					LOG(WARN) << UnrecognisedCommand;
				}
				return {false, {}, {}};
			}
			else
			{
				std::swap(tokens[0], tokens[1]);
				return internalTokenise(tokens, context, true);
			}
		}
	}

	void ConsoleHelper::LoopPrompt()
	{
		std::cout << ConsoleString << CmdColor << std::flush;
	}

} // namespace JSL::Interface
