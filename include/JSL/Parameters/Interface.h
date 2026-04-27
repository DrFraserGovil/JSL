#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <map>
#include <iostream>
namespace JSL::internal::Parameter
{
        std::string_view Normalize(std::string_view s); 


    class Interface
    {
        public:
            static Interface & Get()
            {
                static Interface instance;
                return instance;
            }
            Interface() = default;
            Interface(int argc, char**argv);
            void Parse(int argc, char**argv);
            void Configure(std::filesystem::path configFile, std::string_view configDelimiter);
            void Reset();
            bool IsConfigured();
            bool Contains(std::string_view option) const;
            std::string_view GetOption(std::string_view key) const;
            std::vector<std::string> Commands;
            std::map<std::string,std::string> Options;
            bool Configured = false;
    };
}
