#include <JSL/Interface/Config.h>

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

	Config::Config(std::filesystem::path path, std::string_view configDelim, KeyMapper &keys) : Keys(keys)
	{

		Initialise(configDelim);
		Data["config"] = path.string();
		Parse();
	}

	Config::Config(std::vector<std::filesystem::path> paths, std::string_view configDelim, KeyMapper &keys) : Keys(keys)
	{
		Initialise(configDelim);

		Data["config"] = JSL::String::stitch(paths, ",");
		Parse();
	}
	Config::Config(std::string pathlist, std::string_view configDelim, KeyMapper &keys) : Keys(keys)
	{
		Initialise(configDelim);
		Data["config"] = pathlist;
		Parse();
	}

	void Config::Initialise(std::string_view delim)
	{
		configDelim = delim;
	}

	void Config::Parse()
	{
		auto files = JSL::String::split(Data["config"], ",");
		Data.erase("config");
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
					auto sp = String::split_view(line, configDelim);
					std::string tmp(sp[0]);

					auto key = Keys.CheckAlias(tmp);

					auto val = JSL::String::stitch(sp, 1, sp.size(), configDelim);
					if (Keys.Context.contains(key))
					{
						if (Keys.Context[key] == KeyType::Multivalue)
						{
							if (val.empty())
							{
								Data[key] = "";
							}
							else
							{
								Data[key] += val;
							}
						}
						else
						{
							if (val.empty())
							{
								JSL::internal::FatalError("Bad Config", JSL_LOCATION) << "No value given to config key '" << key << "' in file " << path.string();
							}
							Data[key] = val;
						}
					}
					else
					{
						JSL::internal::FatalError("Bad Config", JSL_LOCATION) << "Unknown configuration key '" << key << "'";
					}
				}
			});
			// now post process any config files that were re-injected
			// files assume locations given relative to the file they were in
			if (Data.contains("config-delim"))
			{
				JSL::internal::FatalError("Bad Configure", JSL_LOCATION) << "The config file " << path << " attempted to redefine config delimiter: this is not allowed";
			}
			if (Data.contains("config"))
			{
				auto tmp = JSL::String::split_view(Data["config"], ",");
				for (auto file : tmp)
				{
					auto canp = fs::canonical(path.parent_path() / file);
					if (!visitedFiles.contains(canp))
					{
						files.push_back(canp);
					}
				}
				Data.erase("config");
			}
		}
	}
} // namespace JSL::Interface
