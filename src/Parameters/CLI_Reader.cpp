#include <JSL/Parameters/CLI_Reader.h>
#include <JSL/FileIO.h>
#include <JSL/Strings/Stitch.h>
#include <cstring>
#include <cctype>
bool ElementIsFlag(char * nextElement)
{
    bool hasDash = (nextElement[0] == '-'); //dashes signify commands, but also negative nos.
    bool isSingleCharacter = (std::strlen(nextElement) == 1);

    if (!hasDash || isSingleCharacter)
    {
        return false; //entries that are not preceeded by a dash, or are a single character long, cannot be command triggers
    }

    return !isdigit(nextElement[1]); //negative numbers are not flags, even though they start with a dash. 
    
}



namespace JSL::internal::Parameter
{
    std::string_view Normalize(std::string_view s)
    {
        const size_t start = s.find_first_not_of('-');
        // If it's all dashes, return an empty view; otherwise, strip them.
        return (start == std::string_view::npos) ? "" : s.substr(start);
    }


    CommandLineReader::CommandLineReader(int argc, char**argv)
    {
        Parse(argc, argv);
    }
    void CommandLineReader::Reset()
    {
        Configured = false;
        Commands.clear();
        Options.clear();
    }
    bool CommandLineReader::IsConfigured()
    {
        return Configured;
    }
    std::string_view CommandLineReader::GetOption(std::string_view key) const
    {
        //trust the user to have already run contains
        std::string skey = static_cast<std::string>(Normalize(key));
        return Options.at(skey);
    }
    bool CommandLineReader::Contains(std::string_view option) const
    {
        return Options.contains(static_cast<std::string>(Normalize(option)));
    }
    void CommandLineReader::Parse(int argc, char**argv)
    {
        Reset();
        //start at 1 to skip the executable name
        int idx = 1;
        bool cmdsFinished = false;
        while (idx < argc)
        {
            std::string_view arg(argv[idx]);
            
            if (arg.starts_with("-"))
            {
                cmdsFinished = true;
                auto narg = Normalize(arg);
                if (idx < argc-1 && !ElementIsFlag(argv[idx+1]))
                {

                    Options[static_cast<std::string>(narg)] = argv[idx+1];
                    idx += 2;
                }
                else
                {
                    Options[static_cast<std::string>(narg)] = "__bool_tag__"; //a flag with no value is treated as a boolean true
                    ++idx;
                }
            }
            else
            {
                if (!cmdsFinished)
                {
                    Commands.emplace_back(arg);
                }
                ++idx;
            }
        }

        std::string configDelim = " ";

        if (Options.contains("config-delim"))
        {
            configDelim = Options["config-delim"];
        }
        if (Options.contains("config"))
        {
            Configure(Options["config"], configDelim);
        }
        Configured = true;
    }

    void CommandLineReader::Configure(std::filesystem::path configFile, std::string_view configDelimiter)
    {
        IO::forSplitLineIn(configFile, configDelimiter, [&](auto linevec)
        {
           if (linevec.size() >= 2)
           {
                //treat the first element as the key, and the rest of the line as the value, rejoining with the configDelimiter in case the value itself contained the delimiter
                Options[static_cast<std::string>(Normalize(linevec[0]))] = JSL::String::stitch(linevec, 1, linevec.size(), configDelimiter);
           }
        });
    }


    
}