#pragma once
#include <deque>

#include <iostream>
#include <string>
#include <charconv>
#include <cstdint>
#include <cstring>
#include <sstream>
#include <optional>
namespace JSL::Display::Format
{
    
    /*!
        @brief Describe which element of the terminal text is being modified
    */
    enum Element{
        //! @brief The colour of the text itself
        Foreground,
        //! @brief The colour of the negative space around the text
        Background,
        //! @brief The style of the rendered text, such as bold, italics, underline etc. 
        TextStyle,
    };

    /*!
        @brief Wrapper around an ANSI string sequence, optimised for inserting into output streams. 
        @warning Converts sequences to a blank string when JSL::Terminal::Environment reports a non-ANSI output stream, preventing garbling of escape sequences.
    */
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



    /*!
        @brief An aggregate container for Command objects, representing a persistent foreground/background/style composite.    
        @details Colours are stored as simple variables; the style commands are stored as a `carousel', allowing up to BufferSize unique commands to be stacked. So long as BufferSize >= 2* number of style commands (1 for activation, 1 for decativation), this allows for arbitrary style combinations. We took the easy way out and did the 2x buffer rather than pair up commands with their inverse - so slightly more memory usage at the advantage of less work on our end!  
    */
    class FormatGroup
    {
        private:
            //! @brief The current foreground colour (single variable as only 1 colour assigned at a time)
            std::optional<Command> Foreground;
            
            //! @brief The current background colour (single variable as only 1 colour assigned at a time)
            std::optional<Command> Background;
            
            //! @brief Adds a style command into the carousel buffer. If an identical command already exists in the buffer, the original instance is erased. 
            void AddBuffer(const Command & cmd);
            
            //! @brief A sequence of unique style commands (inserted from front() to back()). 
            std::deque<Command> StyleBuffer;
        public:
            //! @brief Accumulates the command into the current format, based on the Command::type member. Overrides any existing, conflicting styles.
            //! @param cmd A style command to be added to the current aggregate
            void Add(const Command & cmd);
            
            //! @brief Accumulates the provided group into the current format. Overrides any existing, conflicting styles.
            //! @param cmd A set of styles to be added to the current aggregate
            void Add(const FormatGroup & cmd);

            //! @brief Default constructor; initialised to an `unstyled state'
            //! @details The Foreground and Background are both set to DefaultColour(), and the StyleBuffer is empty.
            FormatGroup();
            
            //! @brief Constructs a group which is default, except for the provided command. 
            //! @details The default constructor is called to create a default state, and then the command is added in.
            //! @param a A formatting Command which determines the initial state of the aggregate
            FormatGroup(const Command & a);

            //! @brief The streaming function, which activates the latent ANSI sequences in the stored Commands. 
            //! @details The commands are inserted in the order [style.front(),...,style.back(), Background, Foreground]
            //! @param os An output stream (typically std::cout)
            //! @param c The format group to be injected into the steam
            friend std::ostream& operator<<(std::ostream& os, const FormatGroup& c);

            //! @brief A stringification operator, returns the aggregated ANSI commands as a pure string
            operator std::string() const;

            //! @brief The maximum size of the StyleCarousel
            //! @details Can be modified at runtime without issue, so exposed as a public member in case users want to tweak it for any reason
            size_t BufferSize = 10;
    };


    /*!
        Matches objects that are constructible in to a FormatGroup, or for which JSL::Format::FormatGroup::Add() works
    */
    template<typename T>
    concept FormatType = std::is_constructible_v<FormatGroup, T> ||requires(FormatGroup& g, T&& t) 
    {
        g.Add(std::forward<T>(t));
    };


    //! @brief Operator overload for adding together two format-like objects
    //! @brief Details functions like FormatGroup(a).Add(b). 
    //! @param a The 'base' format
    //! @param b The format added to the base, overwriting any incompatible formats
    //! @returns An FormatGroup containing the aggregate formats 
    template<FormatType T, FormatType U>
    FormatGroup operator+(const T & a, const U & b)
    {
        FormatGroup out(a);
        out.Add(b);
        return out;
    }
    
    //! @brief Operator overload for compositing together string_views and format groups into a 'formatted string'
    /// @tparam T An object type obeying the FormatType concept
    //! @param a A format group to be applied to the string
    //! @param b A string to be formatted
    //! @warning The format group is *not* terminated; so text streamed after b will have the same format
    //! @returns A string containing the input string, with an ANSI-prefix.
    template<FormatType T>
    std::string operator+(const T & a, std::string_view && b)
    {
        return ((std::string)FormatGroup(a)) + std::string{b};
    }

    //! @brief Operator overload for compositing together a string and a post-fixed format string
    /// @tparam T An object type obeying the FormatType concept
    //! @param a A string which will not be formatted
    //! @param b A format group to be applied to subsequent text
    //! @returns A string containing the input string, with an ANSI-suffix.
    template<FormatType T>
    std::string operator+(std::string_view && b, const T & a)
    {
        return std::string{b} + ((std::string)FormatGroup(a));
    }
        

    
}

namespace JSL
{
    namespace Format = JSL::Display::Format;
}