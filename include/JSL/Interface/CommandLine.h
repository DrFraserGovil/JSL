#pragma once
#include "ParserBase.h"
#include <string>
#include <vector>
namespace JSL::Interface
{
	/*!	A class for tokenising and organising argv array parsed into the system. The class has two modes: a
	  'dumb' mode and a 'contextual' mode, depending on how it is initialised
	  @details We use a command-argument paradigm, where `commands' are plain text (i.e. "push" in a git push),
	  and arguments are dash-initialised values that can either be flags ("-q" for quiet mode),
	  or accept subsequent tokens as input ("-outfile a.out" for setting the output)
	  */
	class CommandLine : public ParserBase
	{
	  public:
		//! The ' commands' which are usually used to indicate high-level changes in behaviour of the code
		std::vector<std::string> Commands;

		/*! @brief Constructs and parses the command line object at initialisation
			The context and alias arguments are used to construct a Interface::KeyMapper object, which determines the correct
			way to assign tokens; this results in a much more powerful and consistent interface
			@param argc The number of arguments to be parsed
			@param argv The argument array
			@param context The ContextMap containing the metadata about accepted commands and how they should be parsed
		*/
		CommandLine(int argc, char **argv, ContextMap context = {});

		/*! Delayed initialisation; no parsing is called.
			@param context The ContextMap containing the metadata about accepted commands and how they should be parsed
		*/
		CommandLine(ContextMap context);

		/*! Executes the parsing routine on the input data
			@param argc The number of arguments to be parsed
			@param argv The argument array
		*/
		void Parse(int argc, char **argv);

		// this just transcludes the documentation of the base clase into this class; has no other effect
		using ParserBase::Parse;
		using ParserBase::UnparsedArguments;

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

		// this just transcludes the documentation of the base clase into this class; has no other effect
		using ParserBase::Keys;
	};

	/*! A union of the CommandLine and Config object
	  @details After performing the commandline parsing, any files passed in as "--config" is immediately parsed, with the values
	  Patched in to the existing arrays, with the command line given priority over the config
	*/
	class ConfigurableCommandLine : public CommandLine
	{
	  public:
		//! @brief The `smart' initialiser
		//! The context and alias arguments are used to construct a Interface::KeyMapper object, which determines the correct
		//! way to assign tokens; this results in a much more powerful and consistent interface
		ConfigurableCommandLine(int argc, char **argv, ContextMap context = {});
	};

} // namespace JSL::Interface
