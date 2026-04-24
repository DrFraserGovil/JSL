#include <JSL/internal/error.h>

#include "JSL/Display/ANSI_Codes.h"
#include <stdexcept>

namespace JSL::internal
{
    FatalError::FatalError(std::string msg) : Summary(msg){
        std::cout << Text::Red << Text::Bold << "\n***JSL Library Error***\n" << std::flush;
    }

    FatalError::~FatalError() noexcept (false)
    {
        std::cout << Text::Reset << "\n" << std::endl;
        throw std::runtime_error(Summary);
    }
        
}