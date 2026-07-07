#include <JSL/Display/Format.h>
#include <JSL/Display/Terminal.h>
#include <JSL/internal/error.h>
#include <algorithm>
#include <charconv>
#include <sstream>
namespace JSL::Display
{

	Format::Format()
	{
		buf[0] = '\0';
		len = 0;
	}

	Format::Format(const std::string &input, Element kind) : type(kind)
	{
		len = input.size();
		std::memcpy(buf, input.c_str(), len);
		buf[len] = '\0';
	}

	Format::Format(uint8_t r, uint8_t g, uint8_t b, Element kind) : type(kind)
	{
		char *ptr = buf;
		auto write_str = [&](const char *s, size_t n) {
			for (size_t i = 0; i < n; ++i) *ptr++ = s[i];
		};

		switch (kind)
		{
		case Foreground:
			write_str("\033[38;2;", 7);
			break;
		case Background:
			write_str("\033[48;2;", 7);
			break;
		default:
			JSL::internal::FatalError("Cannot assign a colour to TextStyle object", JSL_LOCATION);
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

	Format::operator std::string() const
	{
		if (Terminal().IsANSICapable())
		{
			return std::string(buf, len);
		}
		else
		{
			return "";
		}
	}

	std::ostream &operator<<(std::ostream &os, const Format &c)
	{
		if (Terminal().IsANSICapable())
		{
			return os.write(c.buf, c.len);
		}
		else
		{
			return os;
		}
	}

	FormatGroup::FormatGroup()
	{
	}
	FormatGroup::FormatGroup(const Format &a) : FormatGroup()
	{
		Add(a);
	}

	void FormatGroup::Add(const Format &cmd)
	{
		switch (cmd.type)
		{
		case Element::Foreground:
			Foreground = cmd;
			break;
		case Element::Background:
			Background = cmd;
			break;
		default:

			AddBuffer(cmd);
			break;
		}
	}

	void FormatGroup::AddBuffer(const Format &cmd)
	{
		if (StyleBuffer.size() >= BufferSize)
		{
			auto it = std::find(StyleBuffer.begin(), StyleBuffer.end(), cmd);
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

	void FormatGroup::Add(const FormatGroup &group)
	{
		if (group.Foreground) { Add(group.Foreground.value()); };
		if (group.Foreground) { Add(group.Background.value()); };
		for (auto b : group.StyleBuffer)
		{
			AddBuffer(b);
		}
	}

	std::ostream &operator<<(std::ostream &os, const FormatGroup &c)
	{
		os << ResetAll();
		for (auto b : c.StyleBuffer)
		{
			os << b;
		}
		if (c.Background) { os << c.Background.value(); }
		if (c.Foreground) { os << c.Foreground.value(); }
		return os;
	}

	FormatGroup::operator std::string() const
	{
		std::ostringstream os;
		os << *this;
		return os.str();
	}

} // namespace JSL::Display
