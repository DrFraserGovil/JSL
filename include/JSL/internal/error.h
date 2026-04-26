#pragma once
#include <string>
#include <iostream>
#define JSL_LOCATION __FILE__,__func,__LINE__
namespace JSL::internal
{
    class FatalError
    {
        private:
            const std::string Summary;
        public:
            FatalError(std::string msg,int callingLine,const std::string & callingFunction,std::string callingFile);

            template<class T>
            FatalError & operator<<(const T & input)
            {
                std::cout << input << std::flush; //we flush in between elements to force as much out as possible before the crash
                return *this;
            }
            ~FatalError() noexcept (false);
    };
}