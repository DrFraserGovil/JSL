#include <JSL/Parameters/Parameter.h>
#include <JSL/Vectors/Search.h>
#include <JSL/Strings/Cases.h>
namespace JSL
{
    void internal::ParameterBase::Parse(int argc, char** argv)
    {
        Interface & instance = Interface::Get();
        if (!instance.IsConfigured())
        {
            instance.Parse(argc, argv);
        }
        ParseFromCache();
    }

    void internal::ParameterBase::SetTriggers(std::initializer_list<std::string> triggers)
    {
        TriggerList = std::vector<std::string>{triggers};
        ValidateTriggers();
    }

    std::vector<std::string> internal::ParameterBase::GetTriggers() const
    {
        return TriggerList;
    }

    void internal::ParameterBase::ParseFromCache()
    {
        auto & instance = Interface::Get();
        
        for (auto trigger : TriggerList)
        {
            if (instance.Contains(trigger))
            {
                Convert(instance.GetOption(trigger));
                break; //first instance takes priority: trigger list is in order of importance
            }
        }
    }

    void internal::ParameterBase::SetDelimiter(std::string_view vectorDelimiter)
    {
        hasParseDelimiter = true;
        VectorParseDelimiter = (std::string)(vectorDelimiter);
    }

    std::string internal::ParameterBase::TriggerString(bool withDash) const
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

    void internal::ParameterBase::ValidateTriggers()
    {
        for (auto & trigger : TriggerList)
        {
            trigger = Normalize(trigger);
            
            if (trigger.empty())
            {
                internal::FatalError("Parameter trigger cannot be empty", JSL_LOCATION) ;
            }
            if (isdigit(trigger[0]))
            {
                internal::FatalError("Parameter trigger cannot start with a digit: " + trigger, JSL_LOCATION) ;
            }
            int nonAlpha = std::count_if(trigger.begin(),trigger.end(),[](char c){ return !(std::isalnum(c) || (c == '-') || (c == '_')); });
            if (nonAlpha > 0)
            {
                internal::FatalError("Parameter trigger cannot contain non-alphanumeric characters: " + trigger, JSL_LOCATION) ;
            }

            auto & registered = Register();
            if (contains(trigger,registered))
            {
                internal::FatalError("Parameter trigger " + trigger + " is already in use by another Parameter", JSL_LOCATION) ;
            }
            registered.push_back(trigger);
        }
    }
}