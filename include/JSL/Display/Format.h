#pragma once
#include <iostream>
#include <string>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <sstream>
namespace JSL::Format
{
    enum Element{
        ForegroundColour=1<<0,
        BackgroundColour=1<<1,
        Style=1<<2,
        Resetter=1<<3
    };
    struct Command
    {
        char buf[20];
        uint8_t len;
        Element type;
        Command();
        Command(const std::string &input, Element kind);
        Command(uint8_t r, uint8_t g, uint8_t b,const char* control,Element kind);
        friend std::ostream& operator<<(std::ostream& os, const Command& c);
        operator std::string() const;
    };



    class FormatGroup
    {
        private:
            std::pair<bool, Command> Foreground = {false,Command()};
            std::pair<bool, Command> Background= {false,Command()};
            std::pair<bool, Command> Style= {false,Command()};
            bool FalseTransparent = true; //elements set false do not force others
        public:
            void Add(const Command & cmd);
            void Add(const FormatGroup & cmd);
            FormatGroup();
            FormatGroup(const Command & a);
            friend std::ostream& operator<<(std::ostream& os, const FormatGroup& c);
            operator std::string() const;
    };

    FormatGroup operator+(const Command & a,const Command & b);

    //handle addition for any types convertible into format groups
    template<class T, class U>
    FormatGroup operator+(const T & a, const T & b)
    {
        FormatGroup out(a);
        out.Add(b);
        return out;
    }
        
}