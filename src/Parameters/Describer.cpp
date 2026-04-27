#include <JSL/Parameters/Describer.h>
#include <regex>

namespace JSL::Parameter
{
    void Description::HelpPrint(size_t left, size_t mid, size_t right)
    {
        auto left_text = Key;
        
        auto mid_text = "<" + TypeString + ">";
        mid_text = std::regex_replace(mid_text, std::regex("std::"),"");

        auto right_text = TextDescription;
        if (Terminal::IsANSICapable())
        {
            right_text += Format::Italics + Format::Colour(50,50,50) +  "\n[Default: " + DefaultValue + "]" + Format::Reset();
        }
        else
        {
             right_text +="\n[Default: " + DefaultValue + "]";
        }
        
        
        // + Format::Italics + "\n[Default: " + DefaultValue + "]";

        columnPrint({left_text,mid_text,right_text},{left,mid,right}, " ");
    }
}