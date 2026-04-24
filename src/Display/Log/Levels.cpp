#include <JSL/Display/Log/Levels.h>
#include <JSL/internal/error.h>
#include <stdexcept>
#include <string>
LogLevel JSL::Log::LogLevelConvert(int level)
{
    switch(level){
        case 0: 
            return ERROR; break;
        case 1:
            return WARN;break;
        case 2:
            return INFO;break;
        case 3:
            return DEBUG;break;
        default:
            internal::FatalError(std::to_string(level) + "is not a valid logging level");
            return ERROR;
            break;
    }
    //do this rather than a static cast to enforce that enums outside of the ones specified are meaningless   
}