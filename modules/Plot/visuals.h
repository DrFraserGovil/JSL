#pragma once

#include <string>
#include "Range.h"
#include "pipe.h"
#include <functional>
namespace JSL::Plotting::AxisMembers
{
 
    template<typename T>
    struct Holder
    {
        T X;
        T Y;
        T Z;
        T Color;
        Holder(){};
        Holder(T x, T y, T z, T col): X(x), Y(y), Z(z), Color(col){};
        void PrintFunction(std::function<void(std::string axis, T value)> func)
        {
            func("x",X);
            func("y",Y);
            func("z",Z);
            func("cb",Color);
        }
    };
    

    class Display
    {
        public:
            Holder<std::string> Label{"X","Y","Z",""};
            Holder<RangeObject> Range;
            Holder<bool> FormatAsPower;

            void WriteFormatting(internal::GnuplotPipe & pipe)
            {
                Label.PrintFunction([&](auto axis, auto value){
                    pipe << "set " << axis << "label \"" << value << "\"\n"; 
                });

                Range.PrintFunction([&](auto axis, auto value){

                    pipe << "set " << axis << "range " << value.ToString() << "\n";
                });
            }
    };

}