#pragma once

#include "KeyMapper.h"
#include <filesystem>
#include <map>
#include <string_view>
#include <vector>
namespace JSL::Interface
{

	//! @brief A class which parses config files & stores the data in a similar fashion to the JSL::Interface::CommandLine arguments
	//! @details There is a strict assumption that config files have a single entry per line, and are of the form [key][delimiter][data]
	//! @details Comments (C++ inline style) are permitted
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
		//! @param paths The files to read as config files. Files are read sequentially, with the most recent files taking priority
		//! @param configDelim The separator between the [key] and [data] on each line of the file
		//! @param keys A KeyMapper object containing the context and alias for the [key]s found in the data file
		Config(std::string pathlist, std::string_view configDelim, KeyMapper &keys);

		//! The parsed dictionary of all keys and their provided data
		//! @details The equivalent of the Argument field of JSL::Interface::CommandLine
		std::map<std::string, std::string> Data;

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
