#pragma once

#include <string>
#include <vector>
#include <filesystem>
#include <map>
namespace JSL
{
    class Interface
    {
        public:
            static Interface & Get();
            Interface() = default;
            Interface(int argc, char**argv);
            void Parse(int argc, char**argv);
            void Configure(std::filesystem::path configFile, std::string_view configDelimiter);
            void Reset();
            bool IsConfigured();
            bool Contains(std::string_view option) const;
            std::string_view GetOption(std::string_view key) const;
        private:
            std::vector<std::string> Commands;
            std::map<std::string,std::string> Options;
            bool Configured = false;
    };
}
