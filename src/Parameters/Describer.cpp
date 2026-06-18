#include <JSL/Parameters/Describer.h>
#include <JSL/Display/Format.h>
#include <regex>
#include <JSL/Strings/Wrap.h>
namespace JSL::Parameter
{
    void Description::HelpPrint(size_t left, size_t mid, size_t right)
    {
        auto left_text = Key;
        
        auto mid_text = "<" + TypeString + ">";
        mid_text = Display::Colour(0,155,185) +  Name + Display::ResetAll() + "\n" + std::regex_replace(mid_text, std::regex("std::"),"");

        auto right_text = TextDescription;
        
        right_text += "\n" + Display::Italics() + Display::Colour(50,50,50) +  "[Default: " + DefaultValue + "]" + Display::ResetAll();
        
        
        // + Display::Italics + "\n[Default: " + DefaultValue + "]";

        std::cout << String::tableFormat({left_text,mid_text,right_text},{left,mid,right}, " ") << "\n";
    }
}