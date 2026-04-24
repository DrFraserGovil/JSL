#pragma once
#include <cstdint>
#include <string_view> 
#include <JSL/Display/Format.h>
/*
    These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output. 

*/

namespace JSL::Terminal
{
    using CursorCommand = std::string; //has to be string (not view) so the MoveTo command returns persist after being constructed
    constexpr CursorCommand Hide         = "\033[?25l";
    constexpr CursorCommand Show         = "\033[?25h";
    constexpr CursorCommand CursorUp     = "\033[A";
    constexpr CursorCommand Backspace    = "\b \b"; //\b on its own just moves cursor; this moves cursor, erases character, then moves it back again
    constexpr CursorCommand EraseRight   = "\033[0K";
    constexpr CursorCommand EraseLeft    = "\033[1K";
    constexpr CursorCommand ClearLine    = "\033[2K\r";
    constexpr CursorCommand ClearScreen  = "\033[2J\r";
    
    enum Direction {Up =0, Down=1,Right=2,Left=3}; //numbers are fixed as we do some ASCI hackery
    CursorCommand Move(Direction dir, unsigned int steps);
    CursorCommand MoveToColumn(uint32_t column);
}


namespace JSL::Format
{
    const Command ResetAll("\033[0m",Element::Resetter);
    //gets \033[0m, so resets all if passed straight to format; but otherwise can selectively clear from a FormatGroup
    Command Reset(Element sections = Element::Resetter);
    
    const Command Black("\033[30m",ForegroundColour);
    const Command Red("\033[31m",ForegroundColour);
    const Command Green("\033[32m",ForegroundColour);
    const Command Yellow("\033[33m",ForegroundColour);
    const Command Blue("\033[34m",ForegroundColour);
    const Command Purple("\033[35m",ForegroundColour);
    const Command Cyan("\033[36m",ForegroundColour);
    const Command White("\033[37m",ForegroundColour);


    const Command Bold("\033[1m",Style);
    const Command Faint("\033[2m",Style);
    const Command Italics("\033[3m",Style);
    const Command Underline("\033[4m",Style);
    const Command Highlight("\033[7m",Style);
    const Command Strike("\033[9m",Style);


    inline Command Colour(uint8_t r,uint8_t g, uint8_t b)
    {
        return Command(r,g,b,"\033[38;2;",ForegroundColour);
    }

    const Command BgBlack("\033[40m",BackgroundColour);
    const Command BgRed("\033[41m",BackgroundColour);
    const Command BgGreen("\033[42m",BackgroundColour);
    const Command BgYellow("\033[43m",BackgroundColour);
    const Command BgBlue("\033[44m",BackgroundColour);
    const Command BgPurple("\033[45m",BackgroundColour);
    const Command BgCyan("\033[46m",BackgroundColour);
    const Command BgWhite("\033[47m",BackgroundColour);

    inline Command BgColour(uint8_t r,uint8_t g, uint8_t b)
    {
        return Command(r,g,b,"\033[48;2;",BackgroundColour);
    }
}
    

 