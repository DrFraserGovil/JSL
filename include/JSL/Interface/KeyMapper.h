#pragma once
#include <map>
#include <set>
#include <string>
namespace JSL::Interface
{
	enum class KeyType
	{
		Flag,
		Value,
		Multivalue,
		String,
	};

	class KeyMapper
	{
	  public:
		KeyMapper() {};
		KeyMapper(std::map<std::string, KeyType> context, std::map<std::string, std::string> aliases = {});
		std::map<std::string, KeyType> Context;
		std::map<std::string, std::string> Aliases;
		std::string CheckAlias(const std::string &key);
		std::set<std::string> Reset;
	};
} // namespace JSL::Interface
