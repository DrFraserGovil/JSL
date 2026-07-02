#pragma once
#include <JSL/Concepts/ranges.h>
#include <string>
namespace JSL::Interface
{

	//! Enum for dictating the different contexts a parameter value might have; distinguished by how they interact with pre-existing values and differing keys
	enum class KeyType
	{
		//! A Standard key-value flag. A single argument is accepted
		Value,
		//! A boolean flag, which might appear on its own (i.e. -q, -r), or which may be followed by a boolean string (1, true, off)
		Flag,
		//! A key which may accept a series of data points. Successive tokens are consumed until an -arg is found; if values were already found, they are appended (so -q 1 -q 2 is equal to -q 1 2). An empty array clears the current value
		Multivalue,
		//! A single-value, but where the content may have spaces. Successive tokens will be consumed until the next -arg; *unless* the first argument is quote-escaped
		String,
	};

	//! @brief Template function to convert general types to KeyType::Value
	template <typename T>
	struct MapTypeToKey
	{
		static constexpr KeyType value = KeyType::Value;
	};

	//! @brief Template function to convert boolean types to KeyType::Flag
	template <>
	struct MapTypeToKey<bool>
	{
		static constexpr KeyType value = KeyType::Flag;
	};

	//! @brief Template function to convert string types to KeyType::String
	template <>
	struct MapTypeToKey<std::string>
	{
		static constexpr KeyType value = KeyType::String;
	};

	//! @brief Template function to convert container types to KeyType::Multivalue
	template <JSL::Concept::NonStringRange T>
	struct MapTypeToKey<T>
	{
		static constexpr KeyType value = KeyType::Multivalue;
	};

} // namespace JSL::Interface
