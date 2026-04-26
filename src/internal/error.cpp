#include <JSL/internal/error.h>

#include "JSL/Display/Log.h"

#include <stdexcept>



namespace JSL::internal
{
    FatalError::FatalError(std::string msg,int callingLine,const std::string & callingFunction,std::string callingFile) : Summary(msg){
        // Log::Global::Config.

        JSL::Log::Core(ERROR,callingLine,callingFunction,callingFile) << "JSL Library Error:" << msg << "\n";
    }

    FatalError::~FatalError() noexcept (false)
    {
        throw std::runtime_error(Summary);
    }
        
}