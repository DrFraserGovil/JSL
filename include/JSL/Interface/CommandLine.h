#pragma once
#include "KeyMapper.h"
#include <JSL/Strings/ParseTo.h>
#include <map>
#include <string>
#include <vector>
namespace JSL::Interface
{
	//! A class for tokenising and organising argv array parsed into the system. The class has two modes: a
	//! 'dumb' mode and a 'contextual' mode, depending on how it is initialised
	//! @details We use a command-argument paradigm, where `commands' are plain text (i.e. "push" in a git push),
	//! and arguments are dash-initialised values that can either be flags ("-q" for quiet mode),
	//! or accept subsequent tokens as input ("-outfile a.out" for setting the output)
	class CommandLine
	{
	  public:
		//! The ' commands' which are usually used to indicate high-level changes in behaviour of the code
		std::vector<std::string> Commands;
		//! The 'arguments' which are used to set values, and indicate flags to change the internal settings of the code
		std::map<std::string, std::string> Arguments;

		//! @brief The 'dumb' mode initialiser for the CommandLine object
		//! All arguments are assumed to follow a [key] [value] syntax, unless two keys are sequential ([key] [key])
		//! Any non-dash-beginning values that cannot be assigned to a key fall through into the commands
		CommandLine(int argc, char **argv);

		//! @brief The `smart' initialiser for the CommandLine object
		//! The context and alias arguments are used to construct a Interface::KeyMapper object, which determines the correct
		//! way to assign tokens; this results in a much more powerful and consistent interface
		CommandLine(int argc, char **argv, std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases = {});

		template <class T>
		T Parse(std::string id, T defaultValue)
		{
			auto key = Keys.CheckAlias(id); // if the command line was initialised with context & aliases, we get a canonical name
			auto ptr = Arguments.find(key);
			return (ptr != Arguments.end()) ? String::ParseTo<T>(ptr->second) : defaultValue;
		}

		template <class T>
		T Parse(const std::vector<std::string> &ids, T defaultValue)
		{
			// this is presumably used when we *didn't* give full context, so we're manually managing the aliases
			bool found = false;
			T val{};
			for (auto alias : ids)
			{
				auto ptr = Arguments.find(alias);
				if (ptr != Arguments.end())
				{
					if (found) throw std::runtime_error("Duplicate aliases found for " + alias);
					val = String::ParseTo<T>(ptr->second);
					found = true;
				}
			}

			return found ? val : defaultValue;
		}

	  protected:
		//! Used to determine if a value should be assigned to a KeyType::Flag argument
		void FlagCheck(const std::string &key, std::vector<std::string> &vals);
		//! Used to determine if a value should be assigned to a KeyType::Value argument
		void ValueCheck(const std::string &key, std::vector<std::string> &vals);
		//! Used to determine if a value should be assigned to a KeyType::Multivalue argument
		void MultiCheck(const std::string &key, std::vector<std::string> &vals);
		//! Used to determine if a value should be assigned to a KeyType::String argument
		void StringCheck(const std::string &key, std::vector<std::string> &vals);
		//! The name of the last key passed in (before aliasing); used to keep a consistent record and give useful output
		std::string KeyBuffer;
		//! The value which provides the contextual information needed for the 'smart' mode to function
		KeyMapper Keys;
	};

	//! A union of the CommandLine and Config object
	//! @details After performing the commandline parsing, any files passed in as "--config" is immediately parsed, with the values
	//! Patched in to the existing arrays, with the command line given priority over the config
	class ConfigurableCommandLine : public CommandLine
	{
	  public:
		//! @brief The `smart' initialiser
		//! The context and alias arguments are used to construct a Interface::KeyMapper object, which determines the correct
		//! way to assign tokens; this results in a much more powerful and consistent interface
		ConfigurableCommandLine(int argc, char **argv, std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases = {});
	};

} // namespace JSL::Interface
