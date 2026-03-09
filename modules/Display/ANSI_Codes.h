#pragma once
#include <iostream>
#include <string>
#include <charconv>
#include <cstdint>
#include <string_view> 
/*
    These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output. 

*/

namespace JSL
{
    namespace detail
    {
        //! A slightly over-engineered way to rapidly construct arbitrary rgb-colour codes
        struct ColourConstructor
        {
            char buf[20];
            uint8_t len;

            ColourConstructor(uint8_t r, uint8_t g, uint8_t b,const char* type)
            {
                char* ptr = buf;
                auto write_str = [&](const char* s, size_t n)
                {
                    for (size_t i = 0; i < n; ++i) *ptr++ = s[i];
                };

                // write_str("\033[38;2;", 7);
                write_str(type, 7);
                ptr = std::to_chars(ptr, ptr + 3, r).ptr;
                *ptr++ = ';';
                ptr = std::to_chars(ptr, ptr + 3, g).ptr;
                *ptr++ = ';';
                ptr = std::to_chars(ptr, ptr + 3, b).ptr;
                *ptr++ = 'm';
                len = static_cast<uint8_t>(ptr - buf);
            }

            friend std::ostream& operator<<(std::ostream& os, const ColourConstructor& c)
            {
                return os.write(c.buf, c.len);
            }

        };
    }
    using format = std::string_view;

    namespace Cursor
    {

        constexpr format Hide         = "\033[?25l";
        constexpr format Show         = "\033[?25h";
        constexpr format CursorUp     = "\033[A";
        inline format MoveToColumn(uint32_t column)
        {
            return "\033[" + std::to_string(column) + "G";
        }
        
        enum Direction {Up =0, Down=1,Right=2,Left=3};
        inline format Move(Direction dir, unsigned int steps)
        {
            return "\033[" + std::to_string(steps) + (char)(dir+65);
        }
        constexpr format Backspace    = "\b \b"; //\b on its own just moves cursor; this moves cursor, erases character, then moves it back again
        constexpr format EraseRight   = "\033[0K";
        constexpr format EraseLeft    = "\033[1K";
        constexpr format ClearLine    = "\033[2K\r";
        constexpr format ClearScreen  = "\033[2J\r";
    }

    namespace Text 
    {
        constexpr format Reset    = "\033[0m";
        
        constexpr format Black    = "\033[30m";
        constexpr format Red      = "\033[31m";
        constexpr format Green    = "\033[32m";
        constexpr format Yellow   = "\033[33m";
        constexpr format Blue     = "\033[34m";
        constexpr format Purple   = "\033[35m";
        constexpr format Cyan     = "\033[36m";
        constexpr format White    = "\033[37m";
        
       
        constexpr format Bold     = "\033[1m";
        constexpr format Faint    = "\033[2m";
        constexpr format Italics  = "\033[3m";
        constexpr format Underline= "\033[4m";
        constexpr format Highlight= "\033[7m";
        constexpr format Strike   = "\033[9m";


        inline detail::ColourConstructor Colour(uint8_t r,uint8_t g, uint8_t b)
        {
            return detail::ColourConstructor(r,g,b,"\033[38;2;");
        }
    }
    namespace Background
    {
        constexpr format Black    = "\033[40m";
        constexpr format Red      = "\033[41m";
        constexpr format Green    = "\033[42m";
        constexpr format Yellow   = "\033[43m";
        constexpr format Blue     = "\033[44m";
        constexpr format Purple   = "\033[45m";
        constexpr format Cyan     = "\033[46m";
        constexpr format White    = "\033[47m";

        inline detail::ColourConstructor Colour(uint8_t r,uint8_t g, uint8_t b)
        {
            return detail::ColourConstructor(r,g,b,"\033[48;2;");
        }
    }

 
}