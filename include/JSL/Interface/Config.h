#pragma once

#include "KeyMapper.h"
#include <filesystem>
#include <map>
#include <string_view>
#include <vector>
namespace JSL::Interface
{

	class Config
	{

	  public:
		Config(std::filesystem::path path, std::string_view configDelim, KeyMapper &keys);
		Config(std::vector<std::filesystem::path> paths, std::string_view configDelim, KeyMapper &keys);
		Config(std::string pathlist, std::string_view configDelim, KeyMapper &keys);
		std::map<std::string, std::string> Data;

	  private:
		KeyMapper &Keys;
		void Initialise(std::string_view delim);
		std::string configDelim;
		void Parse();
	};
} // namespace JSL::Interface
