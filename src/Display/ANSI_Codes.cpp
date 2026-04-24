#include <JSL/Display/ANSI_Codes.h>
#include <string>
/*
    These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output. 

*/
namespace JSL::Terminal
{
    CursorCommand MoveToColumn(uint32_t column)
    {
        return "\033[" + std::to_string(column) + "G";
    }
    
    CursorCommand Move(Direction dir, unsigned int steps)
    {
        return "\033[" + std::to_string(steps) + (char)(dir+65);
    }
}

namespace JSL::Format
{
  
    Command Reset(Element sections)
    {
        auto cmd =ResetAll;
        cmd.type = (Element)(cmd.type | sections);
        return cmd; 
    }
}
