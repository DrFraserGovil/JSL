#include <JSL/Interface/Context.h>
#include <JSL/internal/error.h>
#include <algorithm>
namespace JSL::Interface
{
	std::string_view normalize(std::string_view s)
	{
		const size_t start = s.find_first_not_of('-');
		// If it's all dashes, return an empty view; otherwise, strip them.
		return (start == std::string_view::npos) ? "" : s.substr(start);
	}

	ContextMap::ContextMap(std::vector<Context> context)
	{
		for (auto c : context)
		{
			AddContext(c);
		}
		Initialised = !Registry.empty();
	}
	ContextMap::ContextMap(std::initializer_list<Context> context)
	{
		for (auto c : std::vector<Context>(context))
		{
			AddContext(c);
		}
	}
	void ContextMap::AddContext(Context input)
	{
		int n = Registry.size();
		Registry.push_back(input);

		for (auto rawname : input.Aliases)
		{
			std::string name(normalize(rawname));
			if (name.empty())
			{
				JSL::internal::FatalError("Bad Alias", JSL_LOCATION) << "The key '" << rawname << "' is an invalid parser ID";
			}
			if (AliasMap.contains(name))
			{
				JSL::internal::FatalError("Duplicate Alias", JSL_LOCATION) << "The key '" << rawname << "' is already in use";
			}
			else
			{
				AliasMap[name] = n;
			}
		}
		Initialised = true;
	}

	bool ContextMap::GetReset(const std::string &key)
	{
		int id = GetContextID(key);
		if (id < 0)
		{
			return false;
		}
		else
		{
			return Registry[id].ResetState;
		}
	}
	void ContextMap::SetReset(const std::string &key)
	{
		int id = GetContextID(key);
		if (id >= 0)
		{
			Registry[id].ResetState = true;
		}
	}

	std::pair<std::string, KeyType> ContextMap::GetCanonical(const std::string &key)
	{
		int id = GetContextID(key);
		if (id < 0)
		{
			return {key, KeyType::Value};
		}
		else
		{
			return {Registry[id].Aliases[0], Registry[id].ParseType}; // we require Aliases.size() > 0 at construction
		}
	}
	int ContextMap::GetContextID(const std::string &key)
	{
		if (!Initialised) { return -1; }

		std::string nkey(normalize(key));
		if (AliasMap.contains(nkey))
		{
			return AliasMap[nkey];
		}
		else
		{
			JSL::internal::FatalError("Bad Key", JSL_LOCATION) << "No parameter called `" << key << "' exists";
			return -1; // dead code but stops clangd from complaining
		}
	}
	std::vector<std::string> ContextMap::GetClusteredKeys(const std::string &key)
	{
		std::string nkey(normalize(key));
		if (AliasMap.contains(nkey) || !Initialised)
		{
			return {key};
		}
		std::vector<std::string> out;
		for (char letter : nkey)
		{
			std::string flag(1, letter);
			bool isGood = AliasMap.contains(flag) && Registry[AliasMap[flag]].ParseType == KeyType::Flag;
			bool isUnique = (std::find(out.begin(), out.end(), flag) == out.end());
			if (isGood && isUnique)
			{
				out.push_back(flag);
			}
			else
			{
				JSL::internal::FatalError("Bad Key", JSL_LOCATION) << "No parameter called `" << key << "' exists";
			}
		}
		return out;
	}

	Context::Context(std::initializer_list<std::string> aliases, KeyType type) : ParseType(type), Aliases(aliases)
	{
		if (Aliases.empty())
		{
			JSL::internal::FatalError("Bad Alias Assignment", JSL_LOCATION) << "Cannot create context map with no names";
		}
	}
	Context::Context(std::vector<std::string> aliases, KeyType type) : ParseType(type), Aliases(aliases)
	{
		if (Aliases.empty())
		{
			JSL::internal::FatalError("Bad Alias Assignment", JSL_LOCATION) << "Cannot create context map with no names";
		}
	}
} // namespace JSL::Interface
