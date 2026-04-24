#include <JSL/Display/Log/Config.h>
#include <JSL/Display/Log/Levels.h>

namespace JSL::Log
{
    ConfigObject::ConfigObject()
    {
        SetLevel(INFO);
        ShowHeaders = true;
        AppendNewline = true;
        TerminalOutput = isatty(fileno(stdout));
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

    namespace Global
    {
        ConfigObject Config;
        std::mutex StreamMutex;
    }

} // namespace JSL::Log

