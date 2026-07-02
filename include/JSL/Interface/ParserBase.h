#pragma once

#include "Context.h"
#include <JSL/Strings/ParseTo.h>
#include <map>
#include <vector>

namespace JSL::Interface
{
	class ParserBase
	{
	  public:
		ParserBase(ContextMap keys) : Keys(keys) {};
		std::map<std::string, std::string> UnparsedArguments;

		/*! @brief Perform the type-conversion from the cached string argument into the required output format
		  @tparam T The type to convert the string into
		  @param id The name (or a known alias) to be retrieved
		  @param defaultValue If ``id`` not found in the tokenized input, this is the value to be returned
		  @returns Either a typed instantiation of the value stored in ArgumentTokens[id], or defaultValue.
		  @throws runtime_error if ArgumentTokens[id] exists, but cannot be parsed into type T. See JSL::String::ParseTo<T>
		*/
		template <class T>
		T Parse(std::string id, T defaultValue)
		{

			auto [key, _] = Keys.GetCanonical(id); // if the command line was initialised with context & aliases, we get a canonical namenormalize
			auto ptr = UnparsedArguments.find(key);
			return (ptr != UnparsedArguments.end()) ? String::ParseTo<T>(ptr->second) : defaultValue;
		}

		/*!	@brief Perform the type-conversion from the cached string argument into the required output format
			@details This alias searches for any of the provided id-aliases. It is therefore best suited to the
			'dumb' mode, where aliases are not already stored in the KeyMapper.
			@tparam T The type to convert the string into
			@param ids A list of names (or aliases) that resolve to the same object
			@param defaultValue If no members of ``ids`` are found in the tokenized input, this is the value to be returned
			@returns Either a typed instantiation of the value stored in ArgumentTokens[id] (for some ``id \in ids`` or defaultValue.
			@throws runtime_error if ArgumentTokens[id] exists, but cannot be parsed into type T. See JSL::String::ParseTo<T>
			@throws runtime_error if multiple entries in the alias list have been assigned by the user.
			To avoid this, use 'smart' mode to resolve alias clashes
		*/
		template <class T>
		T Parse(const std::vector<std::string> &ids, T defaultValue)
		{
			// this is presumably used when we *didn't* give full context, so we're manually managing the aliases
			bool found = false;
			T val{};
			for (auto alias : ids)
			{
				auto ptr = UnparsedArguments.find(alias);
				if (ptr != UnparsedArguments.end())
				{
					if (found) throw std::runtime_error("Duplicate aliases found for " + alias);
					val = String::ParseTo<T>(ptr->second);
					found = true;
				}
			}

			return found ? val : defaultValue;
		}

	  protected:
		//! The stored Key-context and alias information
		ContextMap Keys;
	};

} // namespace JSL::Interface
