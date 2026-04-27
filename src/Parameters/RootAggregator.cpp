#include <JSL/Parameters/RootAggregator.h>
#include <JSL/Vectors/Search.h>
#include <JSL/Display.h>
#include <JSL/FileIO/pipedInput.h>
namespace JSL::Parameter
{

    std::string cmdCapture;
    RootAggregator::RootAggregator()
    {
        Connect(HelpToggle,Help);
        Set(Help, HelpToggle,false,{"h","help"},"Help","Equivalent to the help command");   
        AddCommand("help","Activates the helper function, prints out usage, commands and options, then exits without further computation.");
    }
    RootAggregator::RootAggregator(std::string_view name) : RootAggregator()
    {
        Name = static_cast<std::string>(name);
    }
    void RootAggregator::Parse(int argc, char **argv)
    {
        cmdCapture = std::string{argv[0]};
        Parameter::Aggregator::Parse(argc, argv);
        auto cmds = GetCommands();
        if (Help || JSL::contains("help",cmds))
        {
            MainPrintHelp();
            std::exit(0);
        }
    }

    template<class T, class U>
    std::string fmtwrap(T prefix, std::string text, U suffix)
    {
        if (!Terminal::IsANSICapable()){return text;}
        
        return (std::string)prefix + text + suffix;
    }
    template<class T>
    std::string fmtwrap(T prefix, std::string text)
    {
        if (!Terminal::IsANSICapable()){return text;}
        
        return (std::string)prefix + text + Format::Reset();
    }

    void RootAggregator::MainPrintHelp()
    {
        std::cout << "Usage:\n";
        std::cout << "\t" << cmdCapture  << " [commands] [options]\n";
        if (Terminal::IsANSICapable())
        {
            std::cout << Format::Colour(40,40,40) << Format::Italics;
        }
        std::cout << "Commands are any space separated commands before the first option\nOptions must be indicated by at least one dash\n";
        
        //commands are printed only once
        auto cmds = RegisteredCommands();
        if (!cmds.empty())
        {
            printAsTitle("Available Commands");
            auto defaulter = GetDefaultCommand();
            for (auto &[name,txt] : cmds)
            {
                std::string suffix="";
                if (name == defaulter)
                {
                    suffix = fmtwrap(Format::Colour(50,50,50)," (default)");
                }
                std::string title = fmtwrap(Format::Bold + Format::Blue, name);
                columnPrint({title+suffix,txt},{lw,mw+rw}," ");
            }
        }

        PrintHelp(Name);

    }
}