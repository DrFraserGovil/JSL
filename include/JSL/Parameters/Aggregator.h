#pragma once
#include <JSL/Parameters/Parameter.h>
#include <JSL/Parameters/Describer.h>
#include <vector>
#include <string>
#include <map>
#include <set>
namespace JSL
{
    class ParameterAggregator
    {
        public:
            std::string Name = "Unnamed Settings Group";
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
            void Set(T & value,Parameter<T> & configurer, U defaultValue, std::initializer_list<std::string> triggers,std::string name, std::string description)
            {
                configurer = Parameter<T>(static_cast<T>(defaultValue),triggers);
                Connect(configurer, value);
                SetInfo(configurer,name,description);
            }
            template<class T, class U, class V>
            void Set(T & value,Parameter<T> & configurer, V defaultValue, U triggers,std::string name, std::string description)
            {
                configurer = Parameter<T>(static_cast<T>(defaultValue),std::forward<U>(triggers));
                Connect(configurer, value);
                SetInfo(configurer,name,description);
            }



            void SetParameter(std::string_view trigger, std::string_view value);

            std::string GetParameter(std::string_view trigger);
            
            template<class T>
            T & GetParameter(std::string_view trigger)
            {
                auto found = FindParameter(static_cast<std::string>(trigger));
                if (found)
                {
                    Parameter<T>* typedFound = dynamic_cast<Parameter<T>*>(found);
                    if (typedFound)
                    {
                        return typedFound->Value();
                    }
                    else
                    {
                        internal::FatalError("Parameter type error", JSL_LOCATION) << "Parameter -" << trigger << " exists but is not of the requested type";
                    }
                }
                else
                {
                    internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
                }
                auto dummy = new T(); //unreachable, but silences compiler warning about missing return
                return *dummy;
            }

            void push(std::string_view trigger, std::string_view value);
            void join(std::string_view trigger, std::string_view value);
            void remove(std::string_view trigger, std::string_view value);
            void erase(std::string_view trigger, int pos);

            ParameterDescription GetDescription(std::string_view key);

        protected:
            std::map<std::string,internal::ParameterBase*> RegisteredParameters;
            std::vector<ParameterAggregator*> NestedAggregators;
            std::map<std::string,ParameterDescription> Information;

            internal::ParameterBase* FindParameter(const std::string & key);
            bool TryCluster(std::string_view key, std::string_view value);

            template<class T>
            void SetInfo(Parameter<T> & target, std::string_view name, std::string_view description)
            {
                auto key = target.GetTriggers()[0];
                // Information[key].emplace(target,description,name);
                // Information.emplace({key,{description,name}});
                ParameterDescription d(target,description,name);
                Information[key] = d;
            }
        };
        
    class RootParameterAggregator : public ParameterAggregator
    {
        public:
         void Parse(int argc, char** argv);

        RootParameterAggregator();
            
        protected:
           
            bool Help;
            JSL::Parameter<bool> HelpToggle;
            void PrintHelp();
    };
 
}