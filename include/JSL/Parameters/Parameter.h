#pragma once
#include <JSL/Concepts/ranges.h>
#include <JSL/Strings/MakeFrom.h>
#include <JSL/Strings/ParseTo.h>
#include "CLI_Reader.h"
#include <algorithm>
#include <string>
#include <string_view>
#include <algorithm>
namespace JSL::internal::Parameter
{

    inline std::vector<std::string> & Register()
    {
        static std::vector<std::string> RegisteredSymbols = {"config","config-delim"}; //default symbols that are reserved by the interface
        return RegisteredSymbols;
    }

    class ParameterBase
    {
        public:
            virtual ~ParameterBase() = default;
            void Parse(int argc, char** argv);
            void ParseFromCache();

            void SetTriggers(std::initializer_list<std::string> triggers);
            
            template<class U> //anything that can be converted to a vector of string is a valid trigger type; this allows for multiple triggers to be passed in as a vector, or a single trigger as a string
            void SetTriggers(U && triggers)
            {
                TriggerList = std::vector<std::string>{std::forward<U>(triggers)};
                ValidateTriggers();
            }
            
            std::vector<std::string> GetTriggers() const;

            void SetDelimiter(std::string_view vectorDelimiter);
            std::string TriggerString(bool withDash = true) const;
            virtual std::string ValueString() const = 0;

            virtual void Convert(std::string_view sv) = 0;
            virtual void erase(int pos) = 0;
            virtual void remove(std::string_view value) = 0;
            virtual void push(std::string_view value) = 0;
            virtual void join(std::string_view value) = 0;
        protected:
            std::vector<std::string> TriggerList;
            bool hasParseDelimiter = false;
            std::string VectorParseDelimiter= " ";
            void ValidateTriggers();

    };
}

namespace JSL::Parameter
{
    template<class T>
    class Setting : public internal::Parameter::ParameterBase
    {
        public:
            Setting() = default;
            Setting(T defaultValue, std::initializer_list<std::string> triggers) : InternalValue(defaultValue)
            {
                SetTriggers(triggers);
            }

            Setting(T defaultValue, std::initializer_list<std::string> triggers, int argc, char** argv) : Setting(defaultValue, triggers)
            {
                Parse(argc, argv);
            }

            template<class U>
            Setting(T defaultValue, U && triggers) : InternalValue(defaultValue)
            {
                SetTriggers(triggers);
            }

            template<class U>
            Setting(T defaultValue, U && triggers, int argc, char** argv) : Setting(defaultValue,triggers)
            {
                Parse(argc, argv);
            }

            template<class U>
            static Setting<bool> Toggle(U && triggers)
            {
                return Setting<bool>(false,triggers);
            }
            static Setting<bool> Toggle(std::initializer_list<std::string> triggers)
            {
                return Setting<bool>(false,triggers);
            }
            
            template<class U>
            static Setting<bool> Toggle(U && trigger, int argc, char** argv)
            {
                return Setting<bool>(false,trigger,argc,argv);
            }

            void Connect(T & variable) //reference means we can only connect to a real variable
            {
                ConnectedValue = &variable;
                *ConnectedValue = InternalValue;
            }

            T& Value()
            {
                return InternalValue;
            }
            operator T() const
            {
                return InternalValue;
            }
        
           

            std::string ValueString() const
            {
                return String::makeFrom(InternalValue);
            }
            
            void Convert(std::string_view sv)
            {
                if constexpr (JSL::Concept::is_vector<T>::value)
                {
                    InternalValue = hasParseDelimiter ? String::ParseTo<T>(sv, VectorParseDelimiter) : String::ParseTo<T>(sv);
                }
                else
                {
                    if (hasParseDelimiter)
                    {
                        JSL::internal::FatalError("Parameter parsing error",JSL_LOCATION) << "You cannot pass a vector-delimiter to a non-vector Parameter";
                    }
                    InternalValue = String::ParseTo<T>(sv);
                }
                if (ConnectedValue)
                {
                    *ConnectedValue = InternalValue;
                }
            }

            void push(std::string_view value)
            {
                if constexpr (JSL::Concept::is_vector<T>::value)
                {
                    InternalValue.push_back(String::ParseTo<typename T::value_type>(value));
                    if (ConnectedValue)
                    {
                        *ConnectedValue = InternalValue;
                    }
                }
                else
                {
                    JSL::internal::FatalError("Parameter type error", JSL_LOCATION) << "You cannot push to a non-vector Parameter";
                }
            }
            void join(std::string_view value)
            {
                if constexpr (JSL::Concept::is_vector<T>::value)
                {
                    auto parsed = String::ParseTo<T>(value);
                    InternalValue.insert(InternalValue.end(), parsed.begin(), parsed.end());

                    if (ConnectedValue)
                    {
                        *ConnectedValue = InternalValue;
                    }
                }
                else
                {
                    JSL::internal::FatalError("Parameter type error", JSL_LOCATION) << "You cannot push to a non-vector Parameter";
                }
            }
            void remove(std::string_view value)
            {
                if constexpr (JSL::Concept::is_vector<T>::value)
                {
                    // InternalValue.push_back(String::ParseTo<typename T::value_type>(value));
                    auto dvalue = String::ParseTo<typename T::value_type>(value);
                    std::erase(InternalValue,dvalue);
                    if (ConnectedValue)
                    {
                        *ConnectedValue = InternalValue;
                    }
                }
                else
                {
                    JSL::internal::FatalError("Parameter type error", JSL_LOCATION) << "You cannot remove from a non-vector Parameter";
                }
            }
            void erase(int pos)
            {
                if constexpr (JSL::Concept::is_vector<T>::value)
                {
                    if (InternalValue.empty())
                    {
                        JSL::internal::FatalError("Parameter error", JSL_LOCATION) << "Cannot erase from an empty vector Parameter";
                    }
                    while (pos < 0)
                    {
                        pos += InternalValue.size();
                    }
                    pos = pos % InternalValue.size(); //allow for wrap-around indexing
                    InternalValue.erase(InternalValue.begin() + pos);
                    if (ConnectedValue)
                    {
                        *ConnectedValue = InternalValue;
                    }
                }
                else
                {
                    JSL::internal::FatalError("Parameter type error", JSL_LOCATION) << "You cannot erase from a non-vector Parameter";
                }
            }

        private:
           
            T InternalValue;
            T* ConnectedValue = nullptr;


            
    };


}