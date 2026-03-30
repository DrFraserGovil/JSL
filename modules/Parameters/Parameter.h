#pragma once

#include <vector>
#include <string>
#include <string_view>
#include <cstring>
#include <stdexcept>
#include "../utils/jsl_error.h"
#include "Parsing.h"
#include "../FileIO/FileIO.h"
namespace JSL
{


    template<class T>
    class Parameter
    {
        public:

            bool hasParseDelimiter = false;
            std::string VectorParseDelimiter;

            Parameter(T defaultValue, std::string_view argument) : InternalValue(defaultValue), TriggerString(argument)
            {
            }

            Parameter(T defaultValue, std::string_view argument, std::string_view vectorDelimiter) : Parameter(defaultValue, argument)
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
            }

            Parameter(T defaultValue, std::string_view argument, int argc, char* argv[]) : Parameter(defaultValue, argument)
            {
                Parse(argc, argv);
            }
            Parameter(T defaultValue, std::string_view argument, const std::string & configFile, std::string_view configDelimiter) : Parameter(defaultValue, argument)
            {
                Configure(configFile,configDelimiter);
            }

            Parameter(T defaultValue, std::string_view argument, const std::vector<std::string> & splitString) : Parameter(defaultValue,argument)
            {
                
                ParseSplitLine(splitString," ");
            }

            Parameter(T defaultValue, std::string_view argument, std::string_view vectorDelimiter, int argc, char* argv[]) : Parameter(defaultValue, argument, vectorDelimiter)
            {
                Parse(argc, argv);
            }
            Parameter(T defaultValue, std::string_view argument, std::string_view vectorDelimiter,  const std::string & configFile, std::string_view configDelimiter) : Parameter(defaultValue, argument, vectorDelimiter)
            {
                Configure(configFile,configDelimiter);
            }

            void SetValue(T value, bool confirmSafe = false)
            {
                if (!confirmSafe)
                {
                    std::cout << "JSL WARNING: Parameter -" << TriggerString << " modified to " << MakeString(value) << " outside of standard parsing.";
                }
                InternalValue = value;
                CacheValid = false;
            }

            const T& Value() const
            {
                return InternalValue;
            }

            operator T() const
            {
                return InternalValue;
            }

            //!Iterate through a configuration file, extracting Name/InternalValue pairs and calling Parse() in them. Each Name/InternalValue pair should be on a new line in the file, and separated by the *configDelimiter*. \param configFile The path to the file to open and parse for configuration data \param configDelimiter The delimiter used to separate Name/InternalValue pairs in the cofiguration file
            void Configure(const std::string & configFile, std::string_view configDelimiter)
            {
                forSplitLineIn(configFile, configDelimiter, [&](auto linevec)
                {
                   ParseSplitLine(linevec,configDelimiter);
                });
              
            }

            //!Iterate through the provided commandline args, extracting Name/InternalValue pairs and calling Parse() on them. \param argc The number of arguments passed to the program \param argv[] The argument list (argv[0] is assumed to be the the name of the program, and is ignored)
            void Parse(int argc, char* argv[])
            {
                std::string target = "-" + TriggerString;
                for (int idx = 0; idx < argc; ++idx)
                {
                    if (std::string_view(argv[idx]) == target)
                    {
                        if (idx < argc - 1 && internal::ElementIsValue(argv[idx + 1]))
                        {
                            Convert(argv[idx + 1]);
                        }
                        else if constexpr (std::is_same_v<bool, T>)
                        {
                            InternalValue = true;
                            CacheValid = false;
                        }
                        else
                        {
                            throw std::runtime_error("The parameter " + TriggerString + " requires an accompanying value.");
                        }
                        return;
                    }
                }
               
            }

            std::string ToString(std::string_view argDelimiter = "") const
            {
                 if (!CacheValid)
                {
                    CachedValueString = MakeString(InternalValue);
                    CacheValid = true;
                }
                if (argDelimiter.length() == 0)
                {
                    return CachedValueString;
                }
                else
                {
                    return TriggerString + std::string(argDelimiter) +CachedValueString;
                }
            }


            std::string GetTrigger() const
            {
                return TriggerString;
            }

        private:

            T InternalValue;
            std::string TriggerString;
            mutable std::string CachedValueString;
            mutable bool CacheValid = false;

            template<class U> //should work with either string or string_view
            void ParseSplitLine(std::vector<U> linevec,U delim)
            {
                if (linevec.size() > 0 && linevec[0] == TriggerString)
                {
                    if (linevec.size() == 1)
                    {
                            internal::FatalError("Parameter configuration error")  << "Config parameter " << TriggerString << " requires a value.";
                    }

                    // Reconstruct value if it contained the delimiter
                    std::string concat(linevec[1]);
                    for (size_t i = 2; i < linevec.size(); ++i)
                    {
                        concat += std::string(delim) + std::string(linevec[i]);
                    }
                    Convert(concat);
                }
            }

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

    class Toggle : public Parameter<bool>
    {
        public:
            Toggle(std::string_view argument) : Parameter<bool>(false,argument){};
            Toggle(std::string_view argument, int argc, char* argv[]) : Parameter<bool>(false,argument,argc,argv){};
    };
}