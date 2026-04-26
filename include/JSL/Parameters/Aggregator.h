#pragma once
#include <JSL/Parameters/Parameter.h>
#include <vector>
#include <string>
#include <map>
namespace JSL
{
    class ParameterAggregator
    {
        public:
            void Parse(int argc, char** argv);
            std::vector<std::string> GetCommands();

            template<class T>
            void Connect(Parameter<T> & parameter, T & variable)
            {
                parameter.Connect(variable);
                for (auto trigger : parameter.GetTriggers())
                {
                    RegisteredParameters[trigger] = &parameter;
                }
            }
            void Connect(ParameterAggregator & aggregator)
            {
                NestedAggregators.push_back(&aggregator);
            }

            template<class T, class U>
            void Set(T & value,Parameter<T> & configurer, U defaultValue, std::initializer_list<std::string> triggers)
            {
                configurer = Parameter<T>(static_cast<T>(defaultValue),triggers);
                Connect(configurer, value);
            }
            template<class T, class U, class V>
            void Set(T & value,Parameter<T> & configurer, V defaultValue, U triggers)
            {
                configurer = Parameter<T>(static_cast<T>(defaultValue),std::forward<U>(triggers));
                Connect(configurer, value);
            }

        protected:
            std::map<std::string,internal::ParameterBase*> RegisteredParameters;
            std::vector<ParameterAggregator*> NestedAggregators;

            internal::ParameterBase* FindParameter(const std::string & key);
            bool TryCluster(std::string_view key, std::string_view value);
        };
        
    class RootParameterAggregator : public ParameterAggregator
    {
        public:
         void Parse(int argc, char** argv)
            {
                ParameterAggregator::Parse(argc, argv);
                if (Help)
                {
                    std::cout << "Uh oh!\n";
                    // HelpMessages.print();
                    std::exit(0);
                }
            }

        protected:
            RootParameterAggregator()
            {
                Connect(HelpToggle,Help);
            }

           
            bool Help;
            JSL::Parameter<bool> HelpToggle = Parameter<bool>(false,{"help","h"});

    };
 
}