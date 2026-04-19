#pragma once
#include <iostream>
#include <string>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <string_view> 
/*
    These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output. 

*/

namespace JSL
{
    namespace detail
    {
        enum FormatType{ForegroundColour,BackgroundColour,Style,Reset};
    }
    //! A slightly over-engineered way to rapidly construct arbitrary rgb-colour codes
    struct TerminalFormat
    {
        char buf[20];
        uint8_t len;
        detail::FormatType type;
        TerminalFormat()
        {
            buf[0] = '\0';
            len = 0;
        }
        TerminalFormat(std::string input,detail::FormatType kind) : type(kind) 
        {
            len = input.size();
            std::memcpy(buf, input.c_str(), len);
            buf[len] = '\0';
        }
        TerminalFormat(uint8_t r, uint8_t g, uint8_t b,const char* control,detail::FormatType kind) : type(kind)
        {
            char* ptr = buf;
            auto write_str = [&](const char* s, size_t n)
            {
                for (size_t i = 0; i < n; ++i) *ptr++ = s[i];
            };

            // write_str("\033[38;2;", 7);
            write_str(control, 7);
            ptr = std::to_chars(ptr, ptr + 3, r).ptr;
            *ptr++ = ';';
            ptr = std::to_chars(ptr, ptr + 3, g).ptr;
            *ptr++ = ';';
            ptr = std::to_chars(ptr, ptr + 3, b).ptr;
            *ptr++ = 'm';
            len = static_cast<uint8_t>(ptr - buf);
        }

        friend std::ostream& operator<<(std::ostream& os, const TerminalFormat& c)
        {
            return os.write(c.buf, c.len);
        }
        operator std::string() const {
            return std::string(buf,len);
        }

    };
    
    struct FormatAggregator
    {
        std::pair<bool, TerminalFormat> Foreground = {false,TerminalFormat()};
        std::pair<bool, TerminalFormat> Background= {false,TerminalFormat()};
        std::pair<bool, TerminalFormat> Style= {false,TerminalFormat()};
        bool IsEmpty = true;
        FormatAggregator(){}
        void Add(TerminalFormat input)
        {
            IsEmpty = false;
            if (input.type == detail::Reset)
            {
                Foreground.first = false;
                Background.first = false;
                Style.first = false;
                return;
            }
            if (input.type == detail::ForegroundColour)
            {
                Foreground = {true,input};
                return;
            }
            if (input.type == detail::BackgroundColour)
            {
                Background = {true,input};
                return;
            }
            if (input.type == detail::Style)
            {
                Style = {true,input};
                return;
            }
        }
        friend std::ostream& operator<<(std::ostream& os, const FormatAggregator& c)
        {
            if (c.Foreground.first){os << c.Foreground.second;}
            if (c.Background.first){os << c.Background.second;}
            if (c.Style.first){os << c.Style.second;}
            return os;
        }
    };
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

        const TerminalFormat Reset("\033[0m",detail::Reset);
        const TerminalFormat Black("\033[30m",detail::ForegroundColour);
        
        const TerminalFormat Red("\033[31m",detail::ForegroundColour);
        const TerminalFormat Green("\033[32m",detail::ForegroundColour);
        const TerminalFormat Yellow("\033[33m",detail::ForegroundColour);
        const TerminalFormat Blue("\033[34m",detail::ForegroundColour);
        const TerminalFormat Purple("\033[35m",detail::ForegroundColour);
        const TerminalFormat Cyan("\033[36m",detail::ForegroundColour);
        const TerminalFormat White("\033[37m",detail::ForegroundColour);
        
       
        const TerminalFormat Bold("\033[1m",detail::Style);
        const TerminalFormat Faint("\033[2m",detail::Style);
        const TerminalFormat Italics("\033[3m",detail::Style);
        const TerminalFormat Underline("\033[4m",detail::Style);
        const TerminalFormat Highlight("\033[7m",detail::Style);
        const TerminalFormat Strike("\033[9m",detail::Style);


        inline TerminalFormat Colour(uint8_t r,uint8_t g, uint8_t b)
        {
            return TerminalFormat(r,g,b,"\033[38;2;",detail::ForegroundColour);
        }
        
    }
    namespace Background
    {
        const TerminalFormat Black("\033[40m",detail::BackgroundColour);
        const TerminalFormat Red("\033[41m",detail::BackgroundColour);
        const TerminalFormat Green("\033[42m",detail::BackgroundColour);
        const TerminalFormat Yellow("\033[43m",detail::BackgroundColour);
        const TerminalFormat Blue("\033[44m",detail::BackgroundColour);
        const TerminalFormat Purple("\033[45m",detail::BackgroundColour);
        const TerminalFormat Cyan("\033[46m",detail::BackgroundColour);
        const TerminalFormat White("\033[47m",detail::BackgroundColour);

        inline TerminalFormat Colour(uint8_t r,uint8_t g, uint8_t b)
        {
            return TerminalFormat(r,g,b,"\033[48;2;",detail::BackgroundColour);
        }
    }

 
}