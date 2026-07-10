#include <JSL/Interface/Config.h>

#include "JSL/IO/GetFile.h"
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
#include <set>
namespace JSL::Interface
{
	namespace fs = std::filesystem;

	Config::Config(std::vector<std::string> contents, std::string_view configDelim, ContextMap &keys) : ParserBase(keys)
	{
		Initialise(configDelim);
		ParseFileLines(contents, "config constructor");
	}
	Config::Config(std::filesystem::path path, std::string_view configDelim, ContextMap &keys) : ParserBase(keys)
	{

		Initialise(configDelim);
		UnparsedArguments["config"] = path.string();
		ReadConfigFiles();
	}

	Config::Config(std::vector<std::filesystem::path> paths, std::string_view configDelim, ContextMap &keys) : ParserBase(keys)
	{
		Initialise(configDelim);

		UnparsedArguments["config"] = JSL::String::stitch(paths, ",");
		ReadConfigFiles();
	}
	Config::Config(std::string pathlist, std::string_view configDelim, ContextMap &keys) : ParserBase(keys)
	{
		Initialise(configDelim);
		UnparsedArguments["config"] = pathlist;
		ReadConfigFiles();
	}

	void Config::Initialise(std::string_view delim)
	{
		configDelim = delim;
	}

	void Config::ParseFileLines(std::vector<std::string> &lines, std::string origin)
	{
		for (auto line : lines)
		{

			line = String::trim_view(line, "//"); // trim whitespace and any comments
			if (!line.empty())
			{
				auto sp = String::split_view(line, configDelim);
				std::string tmp(sp[0]);

				auto [key, type] = Keys.GetCanonical(tmp);

				auto val = JSL::String::stitch(sp, 1, sp.size(), configDelim);
				if (type == KeyType::Multivalue)
				{
					if (val.empty())
					{
						UnparsedArguments[key] = "";
					}
					else
					{
						UnparsedArguments[key] += val;
					}
				}
				else
				{
					if (val.empty())
					{
						JSL::internal::FatalError("Bad Config", JSL_LOCATION) << "No value given to config key '" << key << "' in " << origin;
					}
					UnparsedArguments[key] = val;
				}
			}
		}
	}

	void Config::ReadConfigFiles()
	{
		auto files = JSL::String::split(UnparsedArguments["config"], ",");
		UnparsedArguments.erase("config");
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

			auto lines = JSL::IO::getFileLines(path);
			ParseFileLines(lines, path.string());

			// now post process any config files that were re-injected
			// files assume locations given relative to the file they were in
			if (UnparsedArguments.contains("config-delim"))
			{
				JSL::internal::FatalError("Bad Configure", JSL_LOCATION) << "The config file " << path << " attempted to redefine config delimiter: this is not allowed";
			}
			if (UnparsedArguments.contains("config"))
			{
				auto tmp = JSL::String::split_view(UnparsedArguments["config"], ",");
				for (auto file : tmp)
				{
					auto canp = fs::canonical(path.parent_path() / file);
					if (!visitedFiles.contains(canp))
					{
						files.push_back(canp);
					}
				}
				UnparsedArguments.erase("config");
			}
		}
	}
} // namespace JSL::Interface
