#include "JSL/Interface/Config.h"
#include <JSL/Interface/CommandLine.h>
#include <JSL/Log.h>
#include <JSL/Strings/ParseTo.h>
#include <JSL/Strings/Split.h>
#include <JSL/Strings/Stitch.h>
#include <JSL/Strings/Trim.h>
#include <JSL/internal/error.h>
#include <cctype>
#include <cstring>
#include <optional>
#include <set>

// internal implementation functions not exposed to the API
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

namespace JSL::Interface
{

	// CommandLine::CommandLine(int argc, char **argv, std::vector<Context> context) : CommandLine(argc, argv, ContextMap(context))
	// {
	// }
	CommandLine::CommandLine(int argc, char **argv, ContextMap context) : ParserBase(context)
	{
		auto tokens = tokenizeInput(argc, argv);

		Commands = tokens.Simple;
		if (Keys.Initialised)
		{
			Context config({"config", "config-file"}, KeyType::Multivalue);
			Context delim({"config-delim"}, KeyType::String);
			Keys.AddContext(config);
			Keys.AddContext(delim);
		}

		for (auto entry : tokens.UnresolvedKeyVals)
		{
			std::vector<std::string> vals = entry.second;
			KeyBuffer = entry.first; // have to keep this saved before aliasing, so that the user gets errors regarding the value they actually passes, not what it aliased to

			auto clusters = Keys.GetClusteredKeys(KeyBuffer);
			for (auto subflag : clusters)
			{
				auto [key, type] = Keys.GetCanonical(subflag);

				switch (type)
				{
				case KeyType::Flag:
					FlagCheck(key, vals);
					break;
				case KeyType::Value:
					ValueCheck(key, vals);
					break;
				case KeyType::Multivalue:
					MultiCheck(key, vals);
					break;
				case KeyType::String:
					StringCheck(key, vals);
				}
				vals.clear(); // clear this in the case of clustering, we don't reinject multiple times
			}
		}

		std::string delimiter = " ";
		if (UnparsedArguments.contains("config-delim"))
		{
			delimiter = UnparsedArguments["config-delim"];
		}
	}

	void CommandLine::FlagCheck(const std::string &key, std::vector<std::string> &vals)
	{
		if (vals.empty())
		{
			UnparsedArguments[key] = "__bool_tag__";
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
			UnparsedArguments[key] = query;
		}
		else
		{
			// if here, then a command was mis-paired
			UnparsedArguments[key] = "__bool_tag__";
			Commands.push_back(query);
		}
	}
	void CommandLine::ValueCheck(const std::string &key, std::vector<std::string> &vals)
	{
		if (vals.empty())
		{
			if (Keys.Initialised)
			{
				JSL::internal::FatalError("Missing argument", JSL_LOCATION) << "The key '" << KeyBuffer << "' requires an argument: none provided";
			}
			else
			{
				// if in dumb mode, assume this was a boolean flag
				vals.push_back("__bool_tag__");
			}
		}

		UnparsedArguments[key] = vals[0];
		// push any remaining args as commands
		for (size_t i = 1; i < vals.size(); ++i)
		{
			Commands.push_back(vals[i]);
		}
	}
	void CommandLine::MultiCheck(const std::string &key, std::vector<std::string> &vals)
	{
		if (vals.empty())
		{
			Keys.SetReset(key);
			UnparsedArguments[key] = "";
			return;
		}
		if (UnparsedArguments.contains(key) && !UnparsedArguments[key].empty())
		{
			UnparsedArguments[key] += "," + JSL::String::stitch(vals, ",");
		}
		else
		{
			UnparsedArguments[key] = JSL::String::stitch(vals, ",");
		}
	}
	void CommandLine::StringCheck(const std::string &key, std::vector<std::string> &vals)
	{
		if (vals.empty())
		{
			UnparsedArguments[key] = {};
			return;
		}
		if (vals[0].find(' ') != std::string::npos)
		{
			// if the first arg has spaces in it, the user properly escaped it; so any subsequent captured signals are errant commands
			UnparsedArguments[key] = vals[0];
			for (size_t j = 1; j < vals.size(); ++j)
			{
				Commands.push_back(vals[j]);
			}
		}
		else
		{
			// otherwise; we've been told to look for a string: get that string
			UnparsedArguments[key] = JSL::String::stitch(vals, " ");
		}
	}

	ConfigurableCommandLine::ConfigurableCommandLine(int argc, char **argv, ContextMap context) : CommandLine(argc, argv, context)
	{
		if (UnparsedArguments.contains("config"))
		{
			auto it = UnparsedArguments.find("config-delim");
			std::string delim = (it != UnparsedArguments.end()) ? it->second : " ";
			LOG(DEBUG) << "Using " << delim;
			Config C(UnparsedArguments["config"], delim, Keys);

			for (auto &pair : C.UnparsedArguments)
			{
				auto [key, type] = Keys.GetCanonical(pair.first);

				auto &vals = pair.second;

				if (UnparsedArguments.contains(key))
				{

					if (type == KeyType::Multivalue)
					{
						bool resetState = Keys.GetReset(key);
						if (resetState)

						{
							auto trimCLI = String::trim(UnparsedArguments[key]);
							auto trimCon = String::trim(vals);
							if (trimCLI.empty())
							{
								UnparsedArguments[key] = trimCon;
							}
							else
							{
								if (!trimCon.empty())
								{
									UnparsedArguments[key] = vals + "," + UnparsedArguments[key];
								}
							}
						}
					}
				}
				else
				{
					UnparsedArguments[key] = vals;
				}
			}
		}
	}

} // namespace JSL::Interface
