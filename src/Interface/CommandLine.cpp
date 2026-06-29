#include "JSL/Strings/Stitch.h"
#include "JSL/Strings/Trim.h"
#include <JSL/IO/ForLineIn.h>
#include <JSL/Interface/CommandLine.h>
#include <JSL/Log.h>
#include <JSL/Strings/ParseTo.h>
#include <JSL/Strings/Split.h>
#include <JSL/internal/error.h>
#include <cctype>
#include <cstring>
#include <filesystem>
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

	CommandLine::CommandLine(int argc, char **argv, std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases) : Context(context), Aliases(aliases)
	{
		auto tokens = tokenizeInput(argc, argv);

		Commands = tokens.Simple;

		Context["config"] = KeyType::Multivalue;
		Context["config-delim"] = KeyType::String;
		for (auto entry : tokens.UnresolvedKeyVals)
		{
			std::vector<std::string> &vals = entry.second;
			std::string key = CheckKey(entry.first);

			switch (Context[key])
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
		}

		std::string delimiter = " ";
		if (Arguments.contains("config-delim"))
		{
			delimiter = Arguments["config-delim"];
		}
		if (Arguments.contains("config"))
		{
			ParseConfig(delimiter);
		}
	}

	namespace fs = std::filesystem;
	void CommandLine::ParseConfig(std::string_view delimiter)
	{
		auto files = JSL::String::split(Arguments["config"], ",");
		Arguments.erase("config");
		Arguments.erase("config-delim");
		std::set<fs::path> visitedFiles = {};
		for (size_t idx = 0; idx < files.size(); ++idx)
		{
			fs::path path(files[idx]);
			if (!fs::exists(path))
			{
				JSL::internal::FatalError("Bad Config File", JSL_LOCATION) << "Cannot find the config file: " << path;
			}
			else
			{
				visitedFiles.insert(fs::canonical(path));
			}

			IO::forLineIn(path, [&](auto line) {
				line = String::trim_view(line, "//"); // trim whitespace and any comments
				if (!line.empty())
				{
					auto sp = String::split_view(line, delimiter);
					std::string tmp(sp[0]);
					auto key = CheckKey(tmp);

					if (Arguments.contains(key))
					{
						// special case for multivals
						if (Context[key] == KeyType::Multivalue && !ResetMultis.contains(key))
						{
							auto val = JSL::String::stitch(sp, 1, sp.size(), ",");
							Arguments[key] = val + "," + Arguments[key];
						}
					}
					else
					{
						auto val = JSL::String::stitch(sp, 1, sp.size(), delimiter);
						Arguments[key] = val;
					}
				}
			});
			// now post process any config files that were re-injected
			// files assume locations given relative to the file they were in
			if (Arguments.contains("config-delim"))
			{
				JSL::internal::FatalError("Bad Configure", JSL_LOCATION) << "The config file " << path << " attempted to redefine config delimiter: this is not allowed";
			}
			if (Arguments.contains("config"))
			{
				auto tmp = JSL::String::split_view(Arguments["config"], ",");
				for (auto file : tmp)
				{
					auto canp = fs::canonical(path.parent_path() / file);
					if (!visitedFiles.contains(canp))
					{
						files.push_back(canp);
					}
				}
				Arguments.erase("config");
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
			JSL::internal::FatalError("Missing argument", JSL_LOCATION) << "The key '" << KeyBuffer << "' requires an argument: none provided";
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
		if (vals.empty())
		{
			ResetMultis.insert(key);
			Arguments[key] = "";
			return;
		}
		if (Arguments.contains(key) && !Arguments[key].empty())
		{
			Arguments[key] += "," + JSL::String::stitch(vals, ",");
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
			return;
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
	std::string CommandLine::CheckKey(std::string &key)
	{
		std::string out;
		KeyBuffer = key;
		if (Aliases.contains(key))
		{
			out = Aliases[key];
		}
		else
		{
			out = key;
		}

		if (!Context.contains(out))
		{
			JSL::internal::FatalError("Unknown CLI key", JSL_LOCATION) << "The key '" << KeyBuffer << "' is not recognised.";
		}
		return out;
	}

} // namespace JSL::Interface
