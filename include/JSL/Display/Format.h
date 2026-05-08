#pragma once
#include <deque>

#include <iostream>
#include <string>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <sstream>
namespace JSL::Format
{
    
    /*!
        @brief Elements are designed to act as bitmasks so ``ForegroundColour & Resetter`` is interpreted as "Reset the foreground colour (but not the style)".
    */
    enum Element{
        Foreground,
        Background,
        TextStyle,
    };

   
    class Command
    {
        public:

        //! Null-command constructor. Sets len = 0 so it can be ignored.
        Command();

        //! The constructor for the `basic' ANSI codes
        //! @param input An ANSI escape code 
        //! @param kind An indicator as to which of the foreground/background/style the code modifies
        Command(const std::string &input, Element kind);

        //! @brief A constructor for generating an ANSI command for 24 bit colours for either the Foreground or Background
        //! @param r The red value in the range [0,255] 
        //! @param g The green value in the range [0,255] 
        //! @param b The blue value in the range [0,255] 
        //! @param kind Either Element::Foreground or Element::Background, to indicate which element is to be coloured
        //! @throws If kind = Element::Style, as this is meaningless 
        Command(uint8_t r, uint8_t g, uint8_t b,Element kind);

        //! Outputs the corresponding ANSI command string to the stream
        friend std::ostream& operator<<(std::ostream& os, const Command& c);

        //! Casts the ANSI command to a string
        operator std::string() const;

        //! An equality operator that  compares the internal resolved strings
        bool operator==(const Command& other) const
        {
            return static_cast<std::string>(*this) == static_cast<std::string>(other);
        }
        
        private:
        //! Yay friends
        friend class FormatGroup;
        
        //! Tells a FormatGroup which category of command this belongs to 
        Element type;
        //! @brief A c-style buffer for the ANSI sequence
        char buf[20];
        //! @brief The length of the data in the buffer
        uint8_t len;

        // auto operator<=>(const Command & other)
        // {
        //     return static_cast<std::string>(*this) <=> static_cast<std::string>(other);
        // }
        
    };



   
    class FormatGroup
    {
        private:
        Command Foreground;
        Command Background;
        void AddBuffer(const Command & cmd);
        public:
        std::deque<Command> StyleBuffer;
            void Add(const Command & cmd);
            void Add(const FormatGroup & cmd);
            FormatGroup();
            FormatGroup(const Command & a);
            friend std::ostream& operator<<(std::ostream& os, const FormatGroup& c);
            operator std::string() const;
            size_t BufferSize = 10;
    };


    /*
        Defines a `format-like' concept: objects that are constructible in to a FormatGroup, and otherwise behave like them
    */
    template<typename T>
    concept FormatType = std::is_constructible_v<FormatGroup, T> ||requires(FormatGroup& g, T&& t) 
    {
        g.Add(std::forward<T>(t));
    };



    //handle addition for any types convertible into format groups
    FormatGroup operator+(const Command & a,const Command & b);

    template<FormatType T, FormatType U>
    FormatGroup operator+(const T & a, const U & b)
    {
        FormatGroup out(a);
        out.Add(b);
        return out;
    }
    template<FormatType T>
    std::string operator+(const T & a, std::string_view && b)
    {
        return ((std::string)FormatGroup(a)) + std::string{b};
    }
    template<FormatType T>
    std::string operator+(std::string_view && b, const T & a)
    {
        return std::string{b} + ((std::string)FormatGroup(a));
    }
        

    
}