#include <JSL/Parameters/Aggregator.h>
#include <JSL/Parameters/Parameter.h>
#include <JSL/Parameters/Interface.h>
#include <JSL/Display/Log.h>

namespace JSL
{
    void ParameterAggregator::Parse(int argc, char** argv)
    {
        //only parse once!
        auto & instance = internal::Interface::Get();
        instance.Parse(argc, argv);
        
        for (auto [key,value] : instance.Options)
        {
            auto found = FindParameter(key);
            if (found)
            {
                found->Convert(value);
            }
            else
            {
                //option was not registered to any parameter, but was still passed in; this is probably a user error, so we warn about it
                LOG(WARN) << "Warning: unrecognized option -" << key << " with value " << value;
            }
        }
    }
 
    std::vector<std::string> ParameterAggregator::GetCommands()
    {
        auto & instance = internal::Interface::Get();
        return instance.Commands;
    }

    internal::ParameterBase* ParameterAggregator::FindParameter(const std::string & key)
    {
        auto it = RegisteredParameters.find(key);
        if (it != RegisteredParameters.end())
        {
            return it->second;
        }
        for (auto nested : NestedAggregators)
        {
            auto result = nested->FindParameter(key);
            if (result)
            {
                return result;
            }
        }
        return nullptr;
    }
}