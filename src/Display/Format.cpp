#include <JSL/Display/Format.h>
#include <JSL/Display/ANSI_Codes.h>
#include <sstream>
namespace JSL::Format
{


    Command::Command()
    {
        buf[0] = '\0';
        len = 0;
    }
    
    Command::Command(const std::string & input, Element kind)
    {
        len = input.size();
        std::memcpy(buf, input.c_str(), len);
        buf[len] = '\0';
    }

    Command::Command(uint8_t r, uint8_t g, uint8_t b,const char* control,Element kind)
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

    Command::operator std::string() const
    {
        return std::string(buf,len);
    }
    
    std::ostream& operator<<(std::ostream& os, const Command &c )
    {
        return os.write(c.buf, c.len);
    }

    FormatGroup operator+(const Command & a,const Command & b)
    {
        FormatGroup out(a);
        out.Add(b);
        return out;
    }


    FormatGroup::FormatGroup()
    {}
    FormatGroup::FormatGroup(const Command & a)
    {
        Add(a);
    }

    void FormatGroup::Add(const Command & cmd)
    {
        if (cmd.type & Element::Resetter)
        {
            FalseTransparent = false;
            if (cmd.type & Element::ForegroundColour)
            {
                Foreground.first = false;
            }
            if (cmd.type & Element::BackgroundColour)
            {
                Background.first = false;
            }
            if (cmd.type & Element::Style)
            {
                Style.first = false;
            }
        }
        else
        {
            if (cmd.type & Element::ForegroundColour)
            {
                Foreground = {true,cmd};
            }
            if (cmd.type & Element::BackgroundColour)
            {
                Background = {true,cmd};
            }
            if (cmd.type & Element::Style)
            {
                Style = {true,cmd};
            }
        }

    }

    void FormatGroup::Add(const FormatGroup & group)
    {
        //transparency is inherited from right to left
        FalseTransparent = group.FalseTransparent;

        if (group.Foreground.first || FalseTransparent)
        {
            Foreground = group.Foreground;
        }
        if (group.Background.first || FalseTransparent)
        {
            Background = group.Background;
        }
        if (group.Style.first || FalseTransparent)
        {
            Style = group.Style;
        }
    }

    std::ostream & operator<<(std::ostream& os, const FormatGroup& c)
    {
        if (c.FalseTransparent)
        {
            os << ResetAll;
        }
        auto stream = [&](std::pair<bool,Command> q)
        {
            if (q.first){os << q.second;};
        };

        stream(c.Foreground);
        stream(c.Background);
        stream(c.Style);
        return os;
    }

    FormatGroup::operator std::string() const
    {
        std::ostringstream os;
        os << *this;
        return os.str();
    }
}