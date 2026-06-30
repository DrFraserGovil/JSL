#pragma once

#include "KeyMapper.h"
#include <JSL/Strings/ParseTo.h>
#include <filesystem>
#include <map>
#include <string_view>
#include <vector>
namespace JSL::Interface
{

	//! @brief A class which parses config files & stores the data in a similar fashion to the CommandLine arguments
	//! @details The assigned file(s) are read in line-by-line by JSL::IO::forLineIn. Blank lines and comments
	//! (denoted by '//') are stripped out, after which there is a strict assumption that config files have a
	//! single entry per line, and are of the form [key][delimiter][data].
	class Config
	{

	  public:
		//! Initialises a Config file reading from the target file
		//! @param path The file to read as a config file
		//! @param configDelim The separator between the [key] and [data] on each line of the file
		//! @param keys A KeyMapper object containing the context and alias for the [key]s found in the data file
		Config(std::filesystem::path path, std::string_view configDelim, KeyMapper &keys);

		//! Initialises a Config file reading sequentually from the target files
		//! @param paths The files to read as config files. Files are read sequentially, with the most recent files taking priority
		//! @param configDelim The separator between the [key] and [data] on each line of the file
		//! @param keys A KeyMapper object containing the context and alias for the [key]s found in the data file
		Config(std::vector<std::filesystem::path> paths, std::string_view configDelim, KeyMapper &keys);
		//! Initialises a Config file reading sequentually from the target files, which are assumed to be stored as a space-delimted array inside the string
		//! @param pathlist The files to read as config files. Files are read sequentially, with the most recent files taking priority
		//! @param configDelim The separator between the [key] and [data] on each line of the file
		//! @param keys A KeyMapper object containing the context and alias for the [key]s found in the data file
		Config(std::string pathlist, std::string_view configDelim, KeyMapper &keys);

		//! The parsed dictionary of all keys and their provided data
		//! @details The equivalent of the Argument field of JSL::Interface::CommandLine
		std::map<std::string, std::string> Data;

		//! @brief Perform the type-conversion from the cached string argument into the required output format
		//! @tparam T The type to convert the string into
		//! @param id The name (or a known alias) to be retrieved
		//! @param defaultValue If ``id`` not found in the tokenized input, this is the value to be returned
		//! @returns Either a typed instantiation of the value stored in Data[id], or defaultValue.
		//! @throws runtime_error if Data[id] exists, but cannot be parsed into type T. See JSL::String::ParseTo<T>
		template <class T>
		T Parse(std::string id, T defaultValue)
		{
			auto key = Keys.CheckAlias(id); // if the command line was initialised with context & aliases, we get a canonical name
			auto ptr = Data.find(key);
			return (ptr != Data.end()) ? String::ParseTo<T>(ptr->second) : defaultValue;
		}

		//! @brief Perform the type-conversion from the cached string argument into the required output format
		//! @details This alias searches for any of the provided id-aliases. It is therefore best suited to the
		//'dumb' mode, where aliases are not already stored in the KeyMapper.
		//! @tparam T The type to convert the string into
		//! @param ids A list of names (or aliases) that resolve to the same object
		//! @param defaultValue If no members of ``ids`` are found in the tokenized input, this is the value to be returned
		//! @returns Either a typed instantiation of the value stored in Data[id] (for some ``id \in ids`` or defaultValue.
		//! @throws runtime_error if Data[id] exists, but cannot be parsed into type T. See JSL::String::ParseTo<T>
		//! @throws runtime_error if multiple entries in the alias list have been assigned by the user. To avoid this, use 'smart' mode to resolve alias clashes
		template <class T>
		T Parse(const std::vector<std::string> &ids, T defaultValue)
		{
			// this is presumably used when we *didn't* give full context, so we're manually managing the aliases
			bool found = false;
			T val{};
			for (auto alias : ids)
			{
				auto ptr = Data.find(alias);
				if (ptr != Data.end())
				{
					if (found) throw std::runtime_error("Duplicate aliases found for " + alias);
					val = String::ParseTo<T>(ptr->second);
					found = true;
				}
			}

			return found ? val : defaultValue;
		}

	  private:
		//! The stored Key-context and alias information
		KeyMapper Keys;
		//! Performs basic initialising of the struct shared between all the constructors
		//! @param delim THe parameter delimiter
		void Initialise(std::string_view delim);
		//! The initial value given to the file delimiter.
		//! @details Only the first delimiter in a line is used: so "key data and more data"
		std::string configDelim;
		void Parse();
	};
} // namespace JSL::Interface
