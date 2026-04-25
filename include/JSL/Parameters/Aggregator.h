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

        protected:
            // std::vector<internal::ParameterBase*> RegisteredParameters;
            std::map<std::string,internal::ParameterBase*> RegisteredParameters;
            std::vector<ParameterAggregator*> NestedAggregators;

            internal::ParameterBase* FindParameter(const std::string & key);
            
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