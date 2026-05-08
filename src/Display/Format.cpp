#include <JSL/Display/Format.h>
#include <JSL/Display/ANSI_Codes.h>
#include <JSL/internal/error.h>
#include <sstream>
#include <algorithm>
namespace JSL::Format
{


	Command::Command()
	{
		buf[0] = '\0';
		len = 0;
	}
	
	Command::Command(const std::string & input, Element kind) : type(kind)
	{
		len = input.size();
		std::memcpy(buf, input.c_str(), len);
		buf[len] = '\0';
	}

	Command::Command(uint8_t r, uint8_t g, uint8_t b,Element kind)
	{
		char* ptr = buf;
		auto write_str = [&](const char* s, size_t n)
		{
			for (size_t i = 0; i < n; ++i) *ptr++ = s[i];
		};

		switch (kind)
		{
			case Foreground:
				write_str("\033[38;2;",7);
				break;
			case Background:
				write_str("\033[48;2;",7);
			default:
				JSL::internal::FatalError("Cannot assign a colour to TextStyle object",JSL_LOCATION);
				break;
		}

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
	{
		Foreground = DefaultColour();
		Background = DefaultColour(true);
	}
	FormatGroup::FormatGroup(const Command & a) : FormatGroup()
	{
		Add(a);
	}

	void FormatGroup::Add(const Command & cmd)
	{
		switch (cmd.type)
		{
			case Element::Foreground:
				Foreground = cmd;
				break;
			case Element::Background:
				Background  = cmd;
				break;
			case Element::TextStyle:
				AddBuffer(cmd);
				break;
		}
	}

	void FormatGroup::AddBuffer(const Command & cmd)
	{
		if (StyleBuffer.size() >= BufferSize)
		{
			auto it = std::find(StyleBuffer.begin(),StyleBuffer.end(),cmd);
			if (it == StyleBuffer.end())
			{
				StyleBuffer.pop_front(); // no duplicate found, so pop front of queue
			}
			else
			{
				StyleBuffer.erase(it); // remove duplicate
			}
		}
		StyleBuffer.emplace_back(cmd);
	}

	void FormatGroup::Add(const FormatGroup & group)
	{
		Add(group.Foreground);
		Add(group.Background);
		for (auto b : group.StyleBuffer)
		{
			AddBuffer(b);
		}
	}

	std::ostream & operator<<(std::ostream& os, const FormatGroup& c)
	{
		os << Format::ResetAll;
		for (auto b : c.StyleBuffer)
		{
			os << b;
		}
		os << c.Background;
		os << c.Foreground;
		return os;
	}

	FormatGroup::operator std::string() const
	{
		std::ostringstream os;
		os << *this;
		return os.str();
	}

 
}