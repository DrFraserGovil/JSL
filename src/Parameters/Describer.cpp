#include <JSL/Parameters/Describer.h>
#include <regex>
#include <JSL/Strings/FoldLine.h>
namespace JSL::Parameter
{
    void Description::HelpPrint(size_t left, size_t mid, size_t right)
    {
        auto left_text = Key;
        
        auto mid_text = "<" + TypeString + ">";
        mid_text = Format::Colour(0,155,185) +  Name + Format::ResetAll + "\n" + std::regex_replace(mid_text, std::regex("std::"),"");

        auto right_text = TextDescription;
        if (Terminal::IsANSICapable())
        {
            right_text += "\n" + Format::Italics() + Format::Colour(50,50,50) +  "[Default: " + DefaultValue + "]" + Format::ResetAll;
        }
        else
        {
             right_text +="\n[Default: " + DefaultValue + "]";
        }
        
        
        // + Format::Italics + "\n[Default: " + DefaultValue + "]";

        String::columnPrint({left_text,mid_text,right_text},{left,mid,right}, " ");
    }
}