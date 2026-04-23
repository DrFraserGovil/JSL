#pragma once
#include <typeindex>
#include "../Strings/Strings.h"
namespace JSL
{
    class ParameterDescription
    {
        public:
        std::string Name;
        std::string Key;
        std::string TypeString;
        std::type_index Type;
        std::string CurrentValue;
        std::string DefaultValue;
        std::string Description;
        void * RiskyPointer;
            ParameterDescription():Type(typeid(void)){RiskyPointer = nullptr;};
            template<class T>
            ParameterDescription(std::string name, std::string type, std::string key, T & currentValue, T defaultValue, std::string description) : Name(name), Key(key), TypeString(type), Type(typeid(T)), Description(description)
            {
                CurrentValue = MakeString(currentValue);
                DefaultValue = MakeString(defaultValue);
                RiskyPointer = &currentValue;
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