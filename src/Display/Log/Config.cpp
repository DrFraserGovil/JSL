#include <JSL/Display/Log/Config.h>
#include <JSL/Display/Terminal.h>

namespace JSL::Log
{

    Config::Config()
    {
        //do the basic configuration (default values set in header so they're visible to the docs)
        TerminalOutput = Terminal::Global().IsANSICapable();
        AlignSize();
    }

    void Config::SetLevel(int level)
    {
        Level = MakeLevel(level);
    }
    
    void Config::AlignSize(size_t debugReserve)
    {
        // auto S = Terminal::jjkGetDimensions();
        auto & T = Terminal::Global();

        //if debugReserve < 50% the line size, reserve it. Else the terminal is way too small, and we disable linefolding
        LineSize = (T.Columns() > 2*debugReserve) ? T.Columns() - debugReserve : debugReserve;
    }
    
    void Config::DisableTerminal()
    {
        TerminalOutput = false; //set it to false, no matter what
    }


    //Meyers Singleton access pattern; lazy construction of a global variable
    Config & Config::Global()
    {
        static Config config;
        return config;
    }

    

} // namespace JSL::Log

