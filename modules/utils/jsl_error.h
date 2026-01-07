#pragma once

#include <stdexcept>
#include <string>
#include "../Display/ANSI_Codes.h"
namespace JSL
{
    namespace internal
    {
        class FatalError
        {
            private:
                const std::string Summary;
            public:
                FatalError(std::string msg) : Summary(msg){
                    std::cout << Text::Red << Text::Bold << "\n\n***JSL Library Error***\n" << std::flush;
                }

                template<class T>
                FatalError & operator<<(const T & input)
                {
                    std::cout << input << std::flush; //we flush in between elements to force as much out as possible before the crash
                    return *this;
                }
                ~FatalError() noexcept(false)
                {
                    std::cout << Text::Reset << "\n" << std::endl;
                    throw std::runtime_error(Summary);
                }
        };
    }
}