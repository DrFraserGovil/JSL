#pragma once
#include <string>
#include <format>
#include "Range.h"
#include "tic.h"
namespace JSL::Plotting::Properties
{
    class Display;

    class PerAxis
    {
         public:
            bool Active;
            std::string Label = ""; 
            JSL::Plotting::RangeObject Range;
            JSL::Plotting::TicProperties Tics;  
            std::string TimeParsingFormat = "%s"; //tell it how to read the time data
            bool Log = false;
            bool Mirrored = false;
            void LogScale(){Log = true;}
            void Mirror(){Mirrored = true;}
            PerAxis(bool defaultActive,std::string name): Active(defaultActive), Label(name){}

            static PerAxis Default(std::string name)
            {
                return PerAxis(true,name);
            }
            static PerAxis Dormant(std::string name)
            {
                return PerAxis(false,name);
            }
    };

    
}