#pragma once
#include "Interface.h"
#include <algorithm>
#include "Parsing.h"
#include <string>
#include <string_view>
namespace JSL
{
    namespace internal
    {

        inline std::vector<std::string> & Register()
        {
            static std::vector<std::string> RegisteredSymbols;
            return RegisteredSymbols;
        }

    class ParameterBase
    {
        public:
            virtual ~ParameterBase() = default;
            void Parse(int argc, char** argv);
            void ParseFromCache();

            void SetTriggers(std::initializer_list<std::string> triggers)
            {
                TriggerList.assign(triggers.begin(), triggers.end());
                ValidateTriggers();
            }

            template<class U> //anything that can be converted to a vector of string is a valid trigger type; this allows for multiple triggers to be passed in as a vector, or a single trigger as a string
            void SetTriggers(U && triggers)
            {
                TriggerList = std::vector<std::string>{std::forward<U>(triggers)};
                ValidateTriggers();
            }
            
            std::vector<std::string> GetTriggers() const
            {
                return TriggerList;
            }

            void SetDelimiter(std::string_view vectorDelimiter);
            std::string TriggerString(bool withDash = true) const;
            virtual std::string ValueString() const = 0;

            virtual void Convert(std::string_view sv) = 0;
            protected:
            std::vector<std::string> TriggerList;
            bool hasParseDelimiter = false;
            std::string VectorParseDelimiter= " ";
            void ValidateTriggers();
    };

    }
    template<class T>
    class Parameter : public internal::ParameterBase
    {
        public:
            Parameter() = default;
            Parameter(T defaultValue, std::initializer_list<std::string> triggers) : InternalValue(defaultValue)
            {
                SetTriggers(triggers);
            }

            Parameter(T defaultValue, std::initializer_list<std::string> triggers, int argc, char** argv) : Parameter(defaultValue, triggers)
            {
                Parse(argc, argv);
            }

            template<class U>
            Parameter(T defaultValue, U && triggers) : InternalValue(defaultValue)
            {
                SetTriggers(triggers);
            }

            template<class U>
            Parameter(T defaultValue, U && triggers, int argc, char** argv) : Parameter(defaultValue,triggers)
            {
                Parse(argc, argv);
            }

            template<class U>
            static Parameter<bool> Toggle(U && triggers)
            {
                return Parameter<bool>(false,triggers);
            }
            static Parameter<bool> Toggle(std::initializer_list<std::string> triggers)
            {
                return Parameter<bool>(false,triggers);
            }
            
            template<class U>
            static Parameter<bool> Toggle(U && trigger, int argc, char** argv)
            {
                return Parameter<bool>(false,trigger,argc,argv);
            }

            void Connect(T & variable) //reference means we can only connect to a real variable
            {
                ConnectedValue = &variable;
                *ConnectedValue = InternalValue;
            }

            const T& Value() const
            {
                return InternalValue;
            }
            operator T() const
            {
                return InternalValue;
            }
        
           

            std::string ValueString() const
            {
                return MakeString(InternalValue);
            }
            
            void Convert(std::string_view sv)
            {
                if constexpr (internal::is_vector<T>::value)
                {
                    InternalValue = hasParseDelimiter ? ParseTo<T>(sv, VectorParseDelimiter) : ParseTo<T>(sv);
                }
                else
                {
                    if (hasParseDelimiter)
                    {
                        internal::FatalError("Parameter parsing error") << "You cannot pass a vector-delimiter to a non-vector Parameter";
                    }
                    InternalValue = ParseTo<T>(sv);
                }
                if (ConnectedValue)
                {
                    *ConnectedValue = InternalValue;
                }
            }

        private:
           
            T InternalValue;
            T* ConnectedValue = nullptr;


            
    };


}