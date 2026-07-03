#pragma once
#include "KeyTypes.h"
#include <map>
#include <set>
#include <string>
namespace JSL::Interface
{
	//! The metadata associated with a configurable command value
	class Context
	{
	  public:
		//! The expected key type; indicating how tokens should be consumed and distributed
		KeyType ParseType;

		//! Any names (including the `canonical name') which can be used to refer to the parameter
		std::vector<std::string> Aliases;

		//! Becomes true if the parameter recieved a reset signal during parsing; used for correctly back-integrating config files
		bool ResetState = false;

		/*! Constructor for an individual context
			@param aliases A vector of alias names. The first in this list will be the `canonical name` for indexing purposes.
			@param type The KeyType used for determining the parser tokenisaiton strategy
			@throws runtime_error If aliases is empty (requires at least 1 argument)
		*/
		Context(std::vector<std::string> aliases, KeyType type = KeyType::Value);
		/*! Constructor for an individual context
			@param aliases A vector-constructor of alias names. The first in this list will be the `canonical name` for indexing purposes.
			@param type The KeyType used for determining the parser tokenisaiton strategy
			@throws runtime_error If aliases is empty (requires at least 1 argument)
		*/
		Context(std::initializer_list<std::string> aliases, KeyType type = KeyType::Value);
	};

	//! An aggregate map of all the metadata that a given CommandLine/Config may use; allowing for the throwing of errors if an unknown value is passed to it
	class ContextMap
	{
	  public:
		/*!	Creates a context map from the input data
		 *	If no context is provided, enters 'dumb' mode, where no validation occurs
		 *	@param context A list of context objects to be entered into the registry
			@throws runtime_error If duplicate aliases are found in the input context list
		*/
		ContextMap(std::vector<Context> context = {});
		/*!	Creates a context map from the input data
		*	If no context is provided, enters 'dumb' mode, where no validation occurs
		*	@param context A list of context objects to be entered into the registry
		@throws runtime_error If duplicate aliases are found in the input context list
		*/
		ContextMap(std::initializer_list<Context> context);

		/*!	@brief Get the 'canonical name' and parse type of the provided alias
			@details If no context was provided (i.e. in `dumb mode'), this always returns {key,KeyType::Value}, and does not throw an error
			@param key The alias of a key in the Registry
			@returns The canonical name and parse type associated with the alias
			@throws runtime_error If no canonical name exists for the provided alias
		*/
		std::pair<std::string, KeyType> GetCanonical(const std::string &key);

		std::vector<std::string> GetClusteredKeys(const std::string &key);
		/*! Sets the ResetState flag for the associated alias to true
			@details Has no effect if in 'dumb mode'
			@param key The alias of a key in the Registry
			@throws runtime_error if key is not a valid alias
		*/
		void SetReset(const std::string &key);

		/*! Gets the ResetState flag for the associated alias
			@details Always returns false if in dumb mode
			@param key The alias of a key in the Registry
			@returns True if SetReset has previously been called on an associated alias
			@throws runtime_error if key is not a valid alias
		*/
		bool GetReset(const std::string &key);

		/*	Inserts a new context into the Registry
			@param input A new context to be added
			@throws runtime_error If the new context has a duplicate alias name
		*/
		void AddContext(Context input);

		bool Initialised = false;

	  private:
		//! Indicates if a registry has been created

		int GetContextID(const std::string &key);
		//! A list of all stored parameters and their associated contexts
		std::vector<Context> Registry;
		void SetReserved();

		//! A map between alias strings and the index of the associated parameter in the Registry
		std::map<std::string, int> AliasMap;
		std::set<std::string> ReservedAliases;
	};
} // namespace JSL::Interface
