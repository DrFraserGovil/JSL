#pragma once
#include "ParserBase.h"
#include <filesystem>
#include <string_view>
#include <vector>
namespace JSL::Interface
{

	//! @brief A class which parses config files & stores the data in a similar fashion to the CommandLine arguments
	//! @details The assigned file(s) are read in line-by-line by JSL::IO::forLineIn. Blank lines and comments
	//! (denoted by '//') are stripped out, after which there is a strict assumption that config files have a
	//! single entry per line, and are of the form [key][delimiter][data].
	class Config : public ParserBase
	{

	  public:
		/*! Initialises a Config file reading from the target file
			@param path The file to read as a config file
			@param configDelim The separator between the [key] and [data] on each line of the file
			@param keys A KeyMapper object containing the context and alias for the [key]s found in the data file
		*/
		Config(std::filesystem::path path, std::string_view configDelim, ContextMap &keys);

		/*! Initialises a Config file reading sequentually from the target files
			@param paths The files to read as config files. Files are read sequentially, with the most recent files taking priority
			@param configDelim The separator between the [key] and [data] on each line of the file
			@param keys A KeyMapper object containing the context and alias for the [key]s found in the data file
		*/
		Config(std::vector<std::filesystem::path> paths, std::string_view configDelim, ContextMap &keys);

		/*! Initialises a Config file reading sequentually from the target files, which are assumed to be stored as a space-delimted array inside the string
			@param pathlist The files to read as config files. Files are read sequentially, with the most recent files taking priority
			@param configDelim The separator between the [key] and [data] on each line of the file
			@param keys A KeyMapper object containing the context and alias for the [key]s found in the data file
		*/
		Config(std::string pathlist, std::string_view configDelim, ContextMap &keys);

		// this just transcludes the documentation of the base clase into this class; has no other effect
		using ParserBase::Parse;
		using ParserBase::UnparsedArguments;

	  private:
		// this just transcludes the documentation of the base clase into this class; has no other effect
		using ParserBase::Keys;

		/*! Performs basic initialising of the struct shared between all the constructors
			@param delim The parameter delimiter */
		void Initialise(std::string_view delim);

		/*! The initial value given to the file delimiter.
			@details Only the first delimiter in a line is used: so "key data and more data" */
		std::string configDelim;

		//! Performs the actual file reading and parsing of the config files
		void ReadConfigFiles();
	};
} // namespace JSL::Interface
