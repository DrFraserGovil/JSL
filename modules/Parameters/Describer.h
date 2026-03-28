#pragma once

#include "../Strings/Strings.h"
namespace JSL
{
    class ParameterDescription
    {
        public:
        std::string Name;
        std::string Key;
        std::string Type;
        std::string CurrentValue;
        std::string DefaultValue;
        std::string Description;
            template<class T>
            ParameterDescription(std::string name, std::string type, std::string key, T currentValue, T defaultValue, std::string description) : Name(name), Key(key), Description(description), Type(type)
            {
                CurrentValue = MakeString(currentValue);
                DefaultValue = MakeString(defaultValue);
            }

            void Query(std::string string, std::vector<ParameterDescription> & vector)
            {
                if (JSL::insensitiveEquals(string,Name) || JSL::insensitiveEquals(string,Key))
                {
                    vector.push_back(*this);
                }
            }

    };
}