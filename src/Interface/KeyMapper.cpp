#include <JSL/Interface/KeyMapper.h>
#include <JSL/internal/error.h>
namespace JSL::Interface
{
	KeyMapper::KeyMapper(std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases) : Context(context), Aliases(aliases)
	{
	}
	std::string KeyMapper::CheckAlias(const std::string &key)
	{
		{
			std::string out;
			if (Aliases.contains(key))
			{
				out = Aliases[key];
			}
			else
			{
				out = key;
			}

			if (!Context.contains(out))
			{
				JSL::internal::FatalError("Unknown CLI key", JSL_LOCATION) << "The key '" << key << "' is not recognised.";
			}
			return out;
		}
	}
} // namespace JSL::Interface
