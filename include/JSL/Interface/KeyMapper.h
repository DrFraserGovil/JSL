#pragma once
#include <JSL/Concepts/ranges.h>
#include <map>
#include <set>
#include <string>
namespace JSL::Interface
{

	//! Enum for dictating the different contexts a parameter value might have; distinguished by how they interact with pre-existing values and differing keys
	enum class KeyType
	{
		Flag,		//!< A boolean flag, which might appear on its own (i.e. -q, -r), or which may be followed by a boolean string (1, true, off)
		Value,		//!< A Standard key-value flag. A single argument is accepted
		Multivalue, //!< A key which may accept a series of data points. Successive tokens are consumed until an -arg is found; if values were already found, they are appended (so -q 1 -q 2 is equal to -q 1 2). An empty array clears the current value
		String,		//!< A single-value, but where the content may have spaces. Successive tokens will be consumed until the next -arg; *unless* the first argument is quote-escaped
	};
	template <typename T>
	struct MapTypeToKey
	{
		static constexpr KeyType value = KeyType::Value;
	};

	template <>
	struct MapTypeToKey<bool>
	{
		static constexpr KeyType value = KeyType::Flag;
	};

	template <>
	struct MapTypeToKey<std::string>
	{
		static constexpr KeyType value = KeyType::String;
	};

	template <JSL::Concept::NonStringRange T>
	struct MapTypeToKey<T>
	{
		static constexpr KeyType value = KeyType::Multivalue;
	};

	//! A struct which checks the validity of parsed keys against a register of known aliases and expected keys
	class KeyMapper
	{
	  public:
		KeyMapper() {};
		//! Simple constructor
		//! @param context The value to be
		KeyMapper(std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases = {});
		//! The expected key type of every parameter that can be parsed by the system; indicating how tokens should be consumed and distributed
		std::map<std::string, KeyType> Context;
		//! A list of aliases, giving the 'canonical name' for a command.
		std::map<std::string, std::string> Aliases;

		//! Checks a parsed string against the alias list, and returns the canonical name
		//! @param key The input value
		//! @returns The canonical name
		//! throws runtime_error If the canonical name is not within the context array
		std::string CheckAlias(const std::string &key);
		//! A list of MultiValue contexts that have recieved a reset signal; useful for the Configuration portion, when working out which data to append
		std::set<std::string> Reset;

	  private:
		bool Initialised = false;
	};
} // namespace JSL::Interface
