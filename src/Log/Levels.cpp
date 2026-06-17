#include <JSL/Log/Levels.h>
#include <JSL/internal/error.h>
#include <string>
LogLevel JSL::Log::MakeLevel(int level)
{
    if (level > DEBUG || level < ERROR)
    {
        JSL::internal::FatalError(std::to_string(level) + "is not a valid logging level",JSL_LOCATION);
        return ERROR;
    }


    else return static_cast<LogLevel>(level);
}