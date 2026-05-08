#pragma once
#include <cstdint>
#include <string_view> 
#include <JSL/Display/Format.h>

/*
	These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output. 

*/
namespace JSL::Terminal
{

	/*! @brief  Determine if the output stream can interpret ANSI escape sequences
		@details Uses isatty (and _isatty for windows) to determine if stdout can render ANSI commands.
		@returns True if the output is capable of interpreting format commands (i.e. a terminal)
		False if the terminal is plaintext (i.e. a file)
	*/
	bool IsANSICapable();

	//!@brief A basic wrapper to make CursorCommands look more fancy than simple strings, and to remind users that the result of treating them as strings is non-trivial (i.e. the length of these strings does not equate to their length on screen!)
	typedef std::string CursorCommand; 
	
	//! @brief Hides the cursor
	constexpr CursorCommand Hide         = "\033[?25l";
	
	//! @brief Shows the cursor
	constexpr CursorCommand Show         = "\033[?25h";
	
	//! @brief Moves the cursor up one line
	constexpr CursorCommand CursorUp     = "\033[A";

	//! @brief Deletes the previous character (except newlines)
	//!@details \\b on its own just moves cursor; this moves cursor, erases character, then moves it back again
	constexpr CursorCommand Backspace    = "\b \b"; 

	//! @brief Erases all characters to the right of the cursor position
	constexpr CursorCommand EraseAllRight   = "\033[0K";
	//! @brief Erases all characters to the left of the cursor position
	constexpr CursorCommand EraseAllLeft    = "\033[1K";
	
	//! @brief Clears the entire line (but does not move the cursor position)
	constexpr CursorCommand ClearLine    = "\033[2K\r";

	//! @brief Clears the entire terminal (and scrollback buffer)
	constexpr CursorCommand ClearScreen  = "\033[3J\r";
	
	//! @brief Moves the cursor to position (1,1) (the top left of the screen)
	constexpr CursorCommand ResetPosition = "\033[1;1\r";

	//! @brief Specify the direction of terminal movement
	//! @details The fixed numbers allow us to do some ASCII hackery, and convert the enums to relevant chars
	enum Direction {Up =0, Down=1,Right=2,Left=3}; 

	//! @brief Move the cursor a specified number of steps in a given direction
	CursorCommand Move(Direction dir, unsigned int steps);

	//! @brief Moves the cursor to the specified column in the current line (no other text is affected)
	//! @brief Columns are 1-indexed 
	CursorCommand MoveToColumn(uint32_t column);
}


namespace JSL::Format
{
	//! Resets all formatting commands, restoring terminal to default state
	const Command ResetAll=Command("\033[0m",static_cast<Element>(Foreground & Background & TextStyle));
	const Command ResetForeground = Command("\033[39m",Foreground);
	const Command ResetBackground = Command("\033[49m",Background);

	  

	Command Bold(bool active = true);
	Command Faint(bool active = true);
	Command Italics(bool active = true);
	Command Underline(bool active = true);
	Command Highlight(bool active = true);
	Command Strike(bool active = true);
	Command Framed(bool active = true);
	Command Circled(bool active = true);



	Command Black(bool targetBackgroundCol = false);
	Command Blue(bool targetBackgroundCol = false);
	Command Cyan(bool targetBackgroundCol = false);
	Command Green(bool targetBackgroundCol = false);
	Command Purple(bool targetBackgroundCol = false);
	Command Red(bool targetBackgroundCol = false);
	Command Yellow(bool targetBackgroundCol = false);
	Command White(bool targetBackgroundCol = false);
	Command DefaultColour(bool targetBackgroundCol = false);

	inline Command Colour(uint8_t r,uint8_t g, uint8_t b,bool targetBackgroundCol = false)
	{
		return Command(r,g,b,Foreground);
	}

	const Command BgBlack=Command("\033[40m",Background);
	const Command BgRed=Command("\033[41m",Background);
	const Command BgGreen=Command("\033[42m",Background);
	const Command BgYellow=Command("\033[43m",Background);
	const Command BgBlue=Command("\033[44m",Background);
	const Command BgPurple=Command("\033[45m",Background);
	const Command BgCyan=Command("\033[46m",Background);
	const Command BgWhite=Command("\033[47m",Background);

}
	

 