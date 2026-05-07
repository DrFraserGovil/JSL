#include <JSL/Display/Log/Config.h>
#include <JSL/Display/Log/Levels.h>
#include <JSL/Display/ANSI_Codes.h>
#include <JSL/Display/Size.h>

namespace JSL::Log
{
    ConfigObject::ConfigObject()
    {
        SetLevel(INFO);
        ShowHeaders = true;
        AppendNewline = true;
        TerminalOutput = Terminal::IsANSICapable();
        AlignSize();
    }

    void ConfigObject::SetLevel(int level)
    {
        Level = MakeLevel(level);
    }
    
    void ConfigObject::Initialise(int level, bool header)
    {
        AppendNewline =true;
        SetLevel(level);
        ShowHeaders = header;
    }



    void ConfigObject::AlignSize(size_t debugReserve)
    {
        auto S = Terminal::GetDimensions();
        LineSize = std::max(debugReserve, S.Columns - debugReserve);
    }
    
    namespace Global
    {
        ConfigObject & Config()
        {
            static ConfigObject config;
            return config;
        }

    }

} // namespace JSL::Log

