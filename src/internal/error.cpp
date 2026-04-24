#include <JSL/internal/error.h>

#include "JSL/Display/ANSI_Codes.h"
#include <stdexcept>

namespace JSL::internal
{
    FatalError::FatalError(std::string msg) : Summary(msg){
        std::cout << Format::Red + Format::Bold << "\n***JSL Library Error***\n" << std::flush;
    }

    FatalError::~FatalError() noexcept (false)
    {
        std::cout << Format::Reset() << "\n" << std::endl;
        throw std::runtime_error(Summary);
    }
        
}