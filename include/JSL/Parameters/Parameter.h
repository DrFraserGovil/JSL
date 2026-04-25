#pragma once
#include "Interface.h"
#include <string>
#include <string_view>
namespace JSL
{

    template<class T>
    class Parameter
    {
        public:
            template<class U>//anything that can be converted to a vector of string is a valid trigger type; this allows for multiple triggers to be passed in as a vector, or a single trigger as a string
            Parameter(T defaultValue, U triggers) : InternalValue(defaultValue), TriggerList(std::vector<std::string>{triggers})
            {
            }
            template<class U>
            Parameter(T defaultValue, U triggers, int argc, char** argv) : InternalValue(defaultValue), TriggerList(std::vector<std::string>{triggers})
            {
                Parse(argc, argv);
            }

            static Parameter<bool> Toggle(std::string_view trigger)
            {
                return Parameter<bool>(false,trigger);
            }
            static Parameter<bool> Toggle(std::string_view trigger, int argc, char** argv)
            {
                return Parameter<bool>(false,trigger,argc,argv);
            }


            const T& Value() const
            {
                return InternalValue;
            }
            operator T() const
            {
                return InternalValue;
            }
        
            void Parse(int argc, char** argv)
            {
                auto instance = Interface::Get();
                if (!instance.IsConfigured())
                {
                    instance.Parse(argc, argv);
                }
                Reparse();
            }
            void Reparse()
            {
                auto instance = Interface::Get();
                for (auto trigger : TriggerList)
                {
                    if (instance.Contains(trigger))
                    {
                        Convert(instance.GetOption(trigger));
                        break;
                    }
                }
            }
            void SetDelimiter(std::string_view vectorDelimiter)
            {
                if constexpr (internal::is_vector<T>::value)
                {
                    hasParseDelimiter = true;
                    VectorParseDelimiter = (std::string)(vectorDelimiter);
                }
                else
                {
                    internal::FatalError("Parameter parsing error") << "You cannot pass a vector-delimiter to a non-vector Parameter";
                }
                Reparse();
            }

            std::string ValueString() const
            {
                return MakeString(InternalValue);
            }
            std::string TriggerString(bool withDash = true) const
            {
                std::string dash = withDash ? "-" : "";
                std::ostringstream os;
                os << dash << TriggerList[0];
                for (size_t i = 1; i < TriggerList.size(); ++i)
                {
                    os << ", " << dash << TriggerList[i];
                }
                return os.str();
            }

        private:
            std::vector<std::string> TriggerList;
            T InternalValue;
            std::string VectorParseDelimiter;
            bool hasParseDelimiter = false;


            void Convert(std::string_view sv)
            {
                if constexpr (internal::is_vector<T>::value)
                {
                    InternalValue = hasParseDelimiter ? ParseTo<T>(sv, VectorParseDelimiter) : ParseTo<T>(sv);
                }
                else
                {
                    InternalValue = ParseTo<T>(sv);
                }
                CacheValid = false;
            }
    };


}