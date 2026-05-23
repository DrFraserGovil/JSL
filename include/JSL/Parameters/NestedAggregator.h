#pragma once
#include <JSL/Parameters/Parameter.h>
#include <JSL/Parameters/Describer.h>
#include <vector>
#include <string>
#include <map>
#include <set>
namespace JSL::Parameter
{
   
    class NestedAggregator
    {
        public:
            //default spaceship operator
            auto operator<=>(const NestedAggregator&) const = default;

            std::string Name = "Unnamed Settings Group";
        protected:
            void Parse(int argc, char** argv);
           
           

          


            void SetParameter(std::string_view trigger, std::string_view value);

            std::string ShowParameter(std::string_view trigger);
            
            template<class T>
            T & GetParameter(std::string_view trigger)
            {
                auto found = FindParameter(static_cast<std::string>(trigger));
                if (found)
                {
                    Setting<T>* typedFound = dynamic_cast<Setting<T>*>(found);
                    if (typedFound)
                    {
                        return typedFound->Value();
                    }
                    else
                    {
                        throw std::logic_error("Parameter -" + std::string(trigger) + " exists but is not of the requested type");
                    }
                }
                else
                {
                    throw std::logic_error("Parameter -" + std::string(trigger) + "does not exist");
                }
                auto dummy = new T(); //unreachable, but silences compiler warning about missing return
                return *dummy;
            }

            void push(std::string_view trigger, std::string_view value);
            void join(std::string_view trigger, std::string_view value);
            void remove(std::string_view trigger, std::string_view value);
            void erase(std::string_view trigger, int pos);

            Description GetDescription(std::string_view key);
            
            void PrintStructure(int indent,std::string runningTitle);
           

            

        
            template<class T>
            void Connect(Setting<T> & parameter, T & variable)
            {
                parameter.Connect(variable);
                for (auto trigger : parameter.GetTriggers())
                {
                    RegisteredParameters[trigger] = &parameter;
                }
            }
            void NestGroup(NestedAggregator & aggregator,std::string name);

            template<class T, class U>
            void Set(T & value,Setting<T> & configurer, U defaultValue, std::initializer_list<std::string> triggers,std::string name, std::string description)
            {
                configurer = Setting<T>(static_cast<T>(defaultValue),triggers);
                Connect(configurer, value);
                SetInfo(configurer,name,description);
            }
            template<class T, class U, class V>
            void Set(T & value,Setting<T> & configurer, V defaultValue, U triggers,std::string name, std::string description)
            {
                configurer = Setting<T>(static_cast<T>(defaultValue),std::forward<U>(triggers));
                Connect(configurer, value);
                SetInfo(configurer,name,description);
            }



         
             
            void PrintHelp(std::string_view assignedName);
            std::map<std::string,internal::Parameter::ParameterBase*> RegisteredParameters;
            std::map<std::string, NestedAggregator*> NestedAggregators;
            std::map<std::string,Description> Information;

            internal::Parameter::ParameterBase* FindParameter(const std::string & key);
            bool TryCluster(std::string_view key, std::string_view value);

            static void printAsTitle(std::string_view input);
            
            template<class T>
            void SetInfo(Setting<T> & target, std::string_view name, std::string_view description)
            {
                auto key = target.GetTriggers()[0];
                Description d(target,description,name);
                Information[key] = d;
            }
            static const size_t lineLength = 80;
            static const size_t lw = 20;
            static const size_t mw = 15;
            static const size_t rw = lineLength -lw - mw;
        };
        
   
 
}