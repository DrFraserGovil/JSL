#include <JSL/Parameters/Aggregator.h>
#include <JSL/Parameters/Parameter.h>
#include <JSL/Parameters/Interface.h>
#include <JSL/Vectors/Search.h>
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
            SetParameter(key, value);
        }
    }

    void ParameterAggregator::SetParameter(std::string_view trigger, std::string_view value)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            TryConvert(found, value, trigger);
        }
        else
        {
            if (!TryCluster(trigger, value))
            {
                LOG(WARN) << "Unrecognized option -" << trigger << " with value " << value;
            }
        }
    }

    std::string ParameterAggregator::GetParameter(std::string_view trigger)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            return found->ValueString();
        }
        else
        {
            internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
            return ""; //unreachable, but silences compiler warning
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

    void ParameterAggregator::push(std::string_view trigger, std::string_view value)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            found->push(value);
        }
        else
        {
            internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
        }
    }
    void ParameterAggregator::join(std::string_view trigger, std::string_view value)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            found->join(value);
        }
        else
        {
            internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
        }
    }
    void ParameterAggregator::remove(std::string_view trigger, std::string_view value)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            found->remove(value);
        }
        else
        {
            internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
        }
    }
    void ParameterAggregator::erase(std::string_view trigger, int pos)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            found->erase(pos);  
        }
    }

    ParameterDescription ParameterAggregator::GetDescription(std::string_view key)
    {
        auto found = FindParameter(static_cast<std::string>(key));
        if (found)
        {
            auto r = Information[found->GetTriggers()[0]];
            r.CurrentValue = found->ValueString();
            return r;
        }
        internal::FatalError("Not found",JSL_LOCATION);
        return ParameterDescription();
    }

    RootParameterAggregator::RootParameterAggregator()
    {
        Connect(HelpToggle,Help);
        Set(Help, HelpToggle,false,{"h","help"},"Help","Activates the helper function routine");   
    }
    void RootParameterAggregator::Parse(int argc, char **argv)
    {
        ParameterAggregator::Parse(argc, argv);
        auto cmds = GetCommands();
        if (Help || JSL::contains("help",cmds))
        {
            PrintHelp();
            std::exit(0);
        }
    }

    void RootParameterAggregator::PrintHelp()
    {
        LOG(WARN) << "";
    }
}