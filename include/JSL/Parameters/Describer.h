#pragma once
#include <string>
#include <string_view>
#include <JSL/Parameters/Parameter.h>

namespace JSL
{
    class ParameterDescription
    {
        public:
        std::string Name;
        std::string Key;
        std::string TypeString;
        std::string CurrentValue;
        std::string DefaultValue;
        std::string Description;
        bool Found=false;

        ParameterDescription(){}
        template<class T>
        ParameterDescription(Parameter<T> & parameter, std::string_view description, std::string_view name) : Name(name),Description(description) 
        {
            Key = parameter.TriggerString();
            TypeString = typeid(T).name();
            DefaultValue = MakeString(parameter.Value());
            CurrentValue = DefaultValue;
            Found = true;
        }

    };
}