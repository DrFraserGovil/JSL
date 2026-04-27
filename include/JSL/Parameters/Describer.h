#pragma once
#include <string>
#include <string_view>
#include <JSL/Parameters/Parameter.h>
#include <unordered_map>
namespace JSL::Parameter
{

    #define INSERT(type) {typeid(type).name(),#type}
    inline const std::unordered_map<std::string, std::string_view> CommonTypes = {
        INSERT(int), 
        INSERT(double), 
        INSERT(std::string), 
        INSERT(bool),
        INSERT(char),
        INSERT(std::vector<int>),
        INSERT(std::vector<std::string>),
        INSERT(std::vector<double>)
    };

    class Description
    {
        public:
        std::string Name;
        std::string Key;
        std::string TypeString;
        std::string CurrentValue;
        std::string DefaultValue;
        std::string TextDescription;
        bool Found=false;

        Description(){}
        template<class T>
        Description(Setting<T> & parameter, std::string_view description, std::string_view name) : Name(name),TextDescription(description) 
        {
            Key = parameter.TriggerString();

            std::string tname = typeid(T).name();
            if (CommonTypes.contains(tname))
            {
                TypeString = CommonTypes.at(tname);
            }
            else
            {
                TypeString = tname;
            }
            DefaultValue = MakeString(parameter.Value());
            CurrentValue = DefaultValue;
            Found = true;
        }


        void HelpPrint(size_t left, size_t mid, size_t right);

    };
}