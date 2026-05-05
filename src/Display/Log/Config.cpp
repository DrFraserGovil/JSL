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
        Level = LogLevelConvert(level);
    }
    
    void ConfigObject::Initialise(int level, bool header)
    {
        AppendNewline =true;
        SetLevel(level);
        ShowHeaders = header;
    }

    void ConfigObject::SetPrompt(std::string_view prompt)
    {
        if (prompt.empty())
        {
            IncludePrompt = false;
        }
        else
        {
            IncludePrompt = true;
            AppendNewline = true;
            ForceClear = true;
            Prompt = static_cast<std::string>(prompt);
        }
    }
    void ConfigObject::ResetPrompt()
    {
        IncludePrompt = false;
    }

    void ConfigObject::AlignSize(size_t debugReserve)
    {
        auto S = Terminal::GetDimensions();
        DebugLineSize = std::max(debugReserve, S.Columns - debugReserve);
    }
    
    namespace Global
    {
        ConfigObject Config;
        std::mutex StreamMutex;
    }

} // namespace JSL::Log

