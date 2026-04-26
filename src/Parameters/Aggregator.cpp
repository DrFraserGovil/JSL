#include <JSL/Parameters/Aggregator.h>
#include <JSL/Parameters/Parameter.h>
#include <JSL/Parameters/Interface.h>
#include <JSL/Display/Log.h>

namespace JSL
{
    void TryConvert(internal::ParameterBase* parameter, std::string_view value,std::string_view activeTrigger)
    {
        try
        {
            parameter->Convert(value);
        }
        catch (const std::exception & e)
        {
            internal::FatalError("Parameter conversion error", JSL_LOCATION) << "Failed to convert value '" << value << "' for parameter -" << activeTrigger;
        }
    }
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
                TryConvert(found, value, key);
            }
            else
            {

                if (!TryCluster(key, value))
                {
                    LOG(WARN) << "Warning: unrecognized option -" << key << " with value " << value;
                }
            }
        }
    }

    

    bool ParameterAggregator::TryCluster(std::string_view key, std::string_view value)
    {

        std::vector<internal::ParameterBase*> clusterableParameters;
        for (char c : key)
        {
            std::string trigger(1,c);
            auto found = FindParameter(trigger);
            if (found)
            {
                clusterableParameters.push_back(found);
            }
            else
            {
                return false; //if any parameter in the cluster is not found, the whole cluster fails
            }
        }
        for (size_t i = 0; i < clusterableParameters.size(); ++i)
        {
            std::string trigger(1,key[i]);
            TryConvert(clusterableParameters[i], value, trigger); 
        }
        return true;
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