#include <JSL/Display/Format.h>
#include <JSL/Display/Terminal.h>
#include <JSL/internal/error.h>
#include <string>

/*
	These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output.

*/
namespace JSL::Display
{
}

namespace JSL::Display
{
	std::string Reset(Element target)
	{
		if (!Terminal().IsANSICapable())
		{
			return "";
		}
		switch (target)
		{
		case Foreground:
			return "\033[39m";
		case Background:
			return "\033[49m";
		case TextStyle:
			return Bold(false) + Italics(false) + Underline(false) + Highlight(false) + Strike(false);
		default:
			return ResetAll();
		}
	}
	std::string ResetAll()
	{
		if (!Terminal().IsANSICapable())
		{
			return "";
		}
		return "\033[0m";
	}

	Format Style(int style)
	{
		return Format("\033[" + std::to_string(style) + "m", TextStyle);
	}
	Format Style(int style, bool active)
	{
		return Style(style + ((active) ? 0 : 20));
	}

	Format Bold(bool active)
	{
		return (active) ? Style(1) : Style(22); // whilst \0x1b[21m is 'unbold' in some terminals, it is more robust to use \0x1b[22m ('normal intensity')
	}
	Format Italics(bool active)
	{
		return Style(3, active);
	}
	Format Faint(bool active)
	{
		return Style(2, active);
	}
	Format Underline(bool active)
	{
		return Style(4, active);
	}
	Format Highlight(bool active)
	{
		return Style(7, active);
	}
	Format Strike(bool active)
	{
		return Style(9, active);
	}

	// converts type + colour-specific offset into the actual ansi code
	Format getCol(bool targetIsBackground, int offset)
	{
		if (targetIsBackground)
		{
			return Format("\033[" + std::to_string(40 + offset) + "m", Background);
		}
		else
		{
			return Format("\033[" + std::to_string(30 + offset) + "m", Foreground);
		}
	}

	Format Black(bool target)
	{
		return getCol(target, 0);
	}
	Format Blue(bool target)
	{
		return getCol(target, 4);
	}
	Format Cyan(bool target)
	{
		return getCol(target, 6);
	}
	Format Green(bool target)
	{
		return getCol(target, 2);
	}
	Format Purple(bool target)
	{
		return getCol(target, 5);
	}
	Format Red(bool target)
	{
		return getCol(target, 1);
	}
	Format Yellow(bool target)
	{
		return getCol(target, 3);
	}
	Format White(bool target)
	{
		return getCol(target, 7);
	}
	Format DefaultColour(bool target)
	{
		return getCol(target, 9);
	}
} // namespace JSL::Display
