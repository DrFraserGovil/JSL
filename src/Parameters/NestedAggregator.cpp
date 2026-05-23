#include <JSL/Parameters/NestedAggregator.h>
#include <JSL/Display/Terminal.h>
#include <JSL/internal/error.h>
#include <JSL/Vectors/Search.h>
#include <JSL/Display/Log.h>
#include <JSL/Display/Format.h>


namespace JSL::Parameter
{
    void TryConvert(internal::Parameter::ParameterBase* parameter, std::string_view value,std::string_view activeTrigger)
    {
        try
        {
            parameter->Convert(value);
        }
        catch (const std::exception & e)
        {
            JSL::internal::FatalError("Parameter conversion error", JSL_LOCATION) << "Failed to convert value '" << value << "' for parameter -" << activeTrigger << e.what();
        }
    }
    void Parameter::NestedAggregator::Parse(int argc, char** argv)
    {
        //only parse once!
        auto & instance = internal::Parameter::CommandLineReader::Get();
        instance.Parse(argc, argv);
        
        for (auto [key,value] : instance.Options)
        {
            SetParameter(key, value);
        }
    }

    void Parameter::NestedAggregator::SetParameter(std::string_view trigger, std::string_view value)
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

    std::string Parameter::NestedAggregator::ShowParameter(std::string_view trigger)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            return found->ValueString();
        }
        else
        {
            JSL::internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
            return ""; //unreachable, but silences compiler warning
        }
    }


    bool Parameter::NestedAggregator::TryCluster(std::string_view key, std::string_view value)
    {

        std::vector<internal::Parameter::ParameterBase*> clusterableParameters;
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
 
    void Parameter::NestedAggregator::NestGroup(Parameter::NestedAggregator & aggregator,std::string name)
    {
        if (NestedAggregators.contains(name))
        {
            JSL::internal::FatalError("Duplicate aggregator name",JSL_LOCATION) << "A settings aggregator with the name '" << name << "' already exists at this level";
        }

        NestedAggregators[name] = (&aggregator);
    }

   
    
    
    

    internal::Parameter::ParameterBase* Parameter::NestedAggregator::FindParameter(const std::string & key)
    {
        auto it = RegisteredParameters.find(key);
        if (it != RegisteredParameters.end())
        {
            return it->second;
        }
        for (auto nested : NestedAggregators)
        {
            auto result = nested.second->FindParameter(key);
            if (result)
            {
                return result;
            }
        }
        return nullptr;
    }

    void Parameter::NestedAggregator::push(std::string_view trigger, std::string_view value)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            found->push(value);
        }
        else
        {
            JSL::internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
        }
    }
    void Parameter::NestedAggregator::join(std::string_view trigger, std::string_view value)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            found->join(value);
        }
        else
        {
            JSL::internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
        }
    }
    void Parameter::NestedAggregator::remove(std::string_view trigger, std::string_view value)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            found->remove(value);
        }
        else
        {
            JSL::internal::FatalError("Parameter not found", JSL_LOCATION) << "No parameter found for trigger -" << trigger;
        }
    }
    void Parameter::NestedAggregator::erase(std::string_view trigger, int pos)
    {
        auto found = FindParameter(static_cast<std::string>(trigger));
        if (found)
        {
            found->erase(pos);  
        }
    }

    Description Parameter::NestedAggregator::GetDescription(std::string_view key)
    {
        auto found = FindParameter(static_cast<std::string>(key));
        if (found)
        {
            auto r = Information[found->GetTriggers()[0]];
            r.CurrentValue = found->ValueString();
            return r;
        }
        JSL::internal::FatalError("Not found",JSL_LOCATION);
        return Description();
    }
    void Parameter::NestedAggregator::PrintStructure(int indent,std::string runningTitle)
    {
        std::string buffer(indent,'\t');
         for (auto & nest : NestedAggregators)
        {
            std::string newRun = runningTitle + "." + nest.first;
            std::cout << buffer << "- " <<newRun <<"\n";
            nest.second->PrintStructure(indent+1,newRun);
        }
    }


    

    void Parameter::NestedAggregator::printAsTitle(std::string_view input)
    {
        auto fg = Format::Black();
        auto bg = Format::White(true); 
        if (Terminal::Global().IsANSICapable()) //piggyback off the terminal checking if formatting can be used
        {
            auto titleCol =  fg + Format::Italics() + bg;
            std::string buffer = (input.size() < lineLength) ? std::string(lineLength - input.size(),' ') : "";
            std::cout << titleCol << input << buffer << Format::ResetAll() << "\n";
        }
        else
        {
            std::cout << "\n-- " << input << " --\n\n";
        }
        
    }

    void Parameter::NestedAggregator::PrintHelp(std::string_view assignedName)
    {
        if (!RegisteredParameters.empty())
        {
            
            printAsTitle(assignedName);

            for (auto & [name,param] : RegisteredParameters)
            {
                
                bool repeated = false;
                auto keys = param->GetTriggers();
                for (auto key : keys)
                {
                    if (key < name) {repeated = true;break;} //we already processed this
                }
                if (!repeated)
                {
                    auto r = Information[keys[0]];
                    r.CurrentValue = param->ValueString();

                    r.HelpPrint(lw,mw,rw);
                }
            }
        }

        for (auto &[key,nest] : NestedAggregators)
        {
            std::string name = (std::string)assignedName + "." + key;
            nest->PrintHelp(name);
        }


    }

  
}