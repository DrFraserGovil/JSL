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

        columnPrint({left_text,mid_text,right_text},{left,mid,right}, " ");
    }
}