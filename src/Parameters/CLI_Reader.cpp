#include "JSL/Strings/Stitch.h"
#include <JSL/Interface/CLI_Reader.h>
#include <JSL/Strings/ParseTo.h>
#include <JSL/internal/error.h>
#include <cctype>
#include <cstring>
#include <optional>
#include <set>

std::string_view normalize(std::string_view s)
{
	const size_t start = s.find_first_not_of('-');
	// If it's all dashes, return an empty view; otherwise, strip them.
	return (start == std::string_view::npos) ? "" : s.substr(start);
}

std::optional<std::string_view> isKey(std::string_view s)
{
	bool hasDash = s.starts_with('-'); // dashes signify commands, but also negative nos.
	if (!hasDash) { return std::nullopt; }

	std::string_view norm(normalize(s));
	if (norm.empty()) { return std::nullopt; }

	bool decimalFound = false;
	bool digitFound = false;
	for (char l : norm)
	{
		if (std::isdigit(l))
		{
			digitFound = true;
		}
		else
		{
			if (l == '.')
			{
				decimalFound = true;
			}
			else
			{
				return norm;
			}
		}
	}
	if (decimalFound && !digitFound)
	{
		return norm;
	}
	return std::nullopt;
}

namespace JSL::Interface
{

	struct InputTokens
	{
		std::vector<std::string> Simple;
		std::vector<std::pair<std::string, std::vector<std::string>>>
			UnresolvedKeyVals;
	};

	InputTokens tokenizeInput(int argc, char **argv)
	{
		InputTokens out;
		bool tmpActive = false;
		std::pair<std::string, std::vector<std::string>> tmp;
		for (int idx = 1; idx < argc; ++idx)
		{
			std::string_view arg(argv[idx]);

			auto key = isKey(arg);
			if (key)
			{
				if (tmpActive)
				{
					out.UnresolvedKeyVals.push_back(tmp);
				}
				tmpActive = true;
				tmp.first = key.value();
				tmp.second = {};
			}
			else
			{
				if (tmpActive)
				{
					tmp.second.emplace_back(arg);
				}
				else
				{
					out.Simple.emplace_back(arg);
				}
			}
		}
		if (tmpActive)
		{
			out.UnresolvedKeyVals.push_back(tmp);
		}
		return out;
	}

	CommandLine::CommandLine(int argc, char **argv)
	{
		auto tokens = tokenizeInput(argc, argv);
		Commands = tokens.Simple;

		for (auto entry : tokens.UnresolvedKeyVals)
		{
			std::string_view key = entry.first;
			std::vector<std::string> &vals = entry.second;

			std::string value;
			if (vals.empty())
			{
				value = "__bool_tag__";
			}
			else
			{
				// we assume a simple 1-arg-per-key; so anything else must be interleaved commands
				value = vals[0];
				for (size_t j = 1; j < vals.size(); ++j)
				{
					Commands.push_back(vals[j]);
				}
			}

			// the simple lexer works from front to back, overwriting any previous entries, so we don't even do any 'existance' checks
			Arguments.insert_or_assign((std::string)key, value);
		}
	}

	CommandLine::CommandLine(int argc, char **argv, std::map<std::string, KeyTypes> context)
	{
		auto tokens = tokenizeInput(argc, argv);

		Commands = tokens.Simple;

		for (auto entry : tokens.UnresolvedKeyVals)
		{
			std::string key = entry.first;
			std::vector<std::string> &vals = entry.second;

			if (!context.contains(key))
			{
				JSL::internal::FatalError("Unknown CLI key", JSL_LOCATION) << "The key '" << key << "' is not recognised.";
			}

			switch (context[key])
			{
			case Flag:
				FlagCheck(key, vals);
				break;
			case Value:
				ValueCheck(key, vals);
				break;
			case Multivalue:
				MultiCheck(key, vals);
				break;
			case String:
				StringCheck(key, vals);
			}
		}
	}
	void CommandLine::FlagCheck(const std::string &key, std::vector<std::string> &vals)
	{
		if (vals.empty())
		{
			Arguments[key] = "__bool_tag__";
			return;
		}
		// flags only ever accept one arg, so take it
		std::string query = vals[0];

		// push any remaining args as commands
		for (size_t i = 1; i < vals.size(); ++i)
		{
			Commands.push_back(vals[i]);
		}

		// now test if it is possible to parse query as a flag; if so,
		static const std::set<std::string> boolStrings = {"true", "false", "yes", "no", "1", "0", "on", "off"};
		if (boolStrings.contains(query))
		{
			Arguments[key] = query;
		}
		else
		{
			// if here, then a command was mis-paired
			Arguments[key] = "__bool_tag__";
			Commands.push_back(query);
		}
	}
	void CommandLine::ValueCheck(const std::string &key, std::vector<std::string> &vals)
	{
		if (vals.empty())
		{
			JSL::internal::FatalError("Missing argument", JSL_LOCATION) << "The key '" << key << "' requires an argument: none provided";
		}

		Arguments[key] = vals[0];
		// push any remaining args as commands
		for (size_t i = 1; i < vals.size(); ++i)
		{
			Commands.push_back(vals[i]);
		}
	}
	void CommandLine::MultiCheck(const std::string &key, std::vector<std::string> &vals)
	{
		// can't do an empty check as empty strings are valid input, and "" is stripped out
		if (Arguments.contains(key))
		{
			if (vals.empty())
			{
				Arguments.erase(key);
			}
			else
			{

				Arguments[key] += "," + JSL::String::stitch(vals, ",");
			}
		}
		else
		{

			Arguments[key] = JSL::String::stitch(vals, ",");
		}
	}
	void CommandLine::StringCheck(const std::string &key, std::vector<std::string> &vals)
	{
		if (vals.empty())
		{
			Arguments[key] = {};
		}
		if (vals[0].find(' ') != std::string::npos)
		{
			// if the first arg has spaces in it, the user properly escaped it; so any subsequent captured signals are errant commands
			Arguments[key] = vals[0];
			for (size_t j = 1; j < vals.size(); ++j)
			{
				Commands.push_back(vals[j]);
			}
		}
		else
		{
			// otherwise; we've been told to look for a string: get that string
			Arguments[key] = JSL::String::stitch(vals, " ");
		}
	}

} // namespace JSL::Interface
