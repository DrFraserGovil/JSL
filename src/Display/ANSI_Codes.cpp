#include <JSL/Display/ANSI_Codes.h>
#include <JSL/Display/Terminal.h>
#include <string>
#include <JSL/internal/error.h>


/*
	These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output. 

*/
namespace JSL::Terminal
{
	CursorCommand MoveToColumn(uint32_t column)
	{
		column = column > 0 ? column : 1; //ensure that 0-indexing doesn't cause issues as columns are 1 indexed
		return "\033[" + std::to_string(column) + "G";
	}
	
	CursorCommand Move(Direction dir, unsigned int steps)
	{
		return "\033[" + std::to_string(steps) + (char)(dir+65);
	}
}

namespace JSL::Format
{	
	std::string Reset(Element target)
	{
		if (!Terminal::Global().IsANSICapable())
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
		if (!Terminal::Global().IsANSICapable())
		{
			return "";
		}
		return "\033[0m";
	}

	Command Style(int style)
    {
        return Command("\033[" + std::to_string(style  ) + "m",TextStyle);
    }
	Command Style(int style,bool active)
    {
        return Style(style + ((active) ? 0 : 20));
    }

	Command Bold(bool active)
	{
		return (active) ? Style(1) : Style(22); //whilst \0x1b[21m is 'unbold' in some terminals, it is more robust to use \0x1b[22m ('normal intensity')

	}
	Command Italics(bool active)
	{
		return Style(3,active);
	}
	Command Faint(bool active)
	{
		return Style(2,active);
	}
	Command Underline(bool active)
	{
		return Style(4,active);
	}
	Command Highlight(bool active)
	{
		return Style(7,active);
	}
	Command Strike(bool active)
	{
		return Style(9,active);
	}


	//converts type + colour-specific offset into the actual ansi code
	Command getCol(bool targetIsBackground, int offset)
	{
		if (targetIsBackground)
		{
			return Command("\033[" + std::to_string(40 + offset) + "m",Background);
		}
		else
		{
			return Command("\033[" + std::to_string(30 + offset) + "m",Foreground);
		}
	}

	Command Black(bool target)
	{
		return getCol(target,0);
	}
	Command Blue(bool target)
	{
		return getCol(target,4);
	}
	Command Cyan(bool target)
	{
		return getCol(target,6);
	}
	Command Green(bool target)
	{
		return getCol(target,2);
	}
	Command Purple(bool target)
	{
		return getCol(target,5);
	}
	Command Red(bool target)
	{
		return getCol(target,1);
	}
	Command Yellow(bool target)
	{
		return getCol(target,3);
	}
	Command White(bool target)
	{
		return getCol(target,7);
	}
	Command DefaultColour(bool target)
	{
		return getCol(target,9);
	}
}
