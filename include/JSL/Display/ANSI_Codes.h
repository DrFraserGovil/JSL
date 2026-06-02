#pragma once
#include <cstdint>
#include <string_view> 
#include <JSL/Display/Format.h>
/*
	These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output. 

*/

namespace JSL::Display::Terminal
{
	//!@brief A basic wrapper to make CursorCommands look more fancy than simple s, and to remind users that the result of treating them as strings is non-trivial (i.e. the length of these strings does not equate to their length on screen!)
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

namespace JSL
{
	namespace Terminal = JSL::Display::Terminal;
}

namespace JSL::Display::Format
{


	/// @brief Returns the specified target back to its default state, as specified by the terminal settings
	/// @param target The element to be reset to default (does not support bitmasking)
	/// @return A string representing the ANSI sequence
	/// @details This returns a string (and not a Command or FormatGroup) specifically because Resets don't play well with the Group infrastructure. A reset is to be applied separately from a FormatGroup
	std::string Reset(Element target);

	/// @brief Returns the terminal back to its default state, as specified by the terminal settings
	/// @return A string representing the ANSI `default' sequence
	/// @details This returns a string (and not a Command or FormatGroup) specifically because Resets don't play well with the Group infrastructure. A reset is to be applied separately from a FormatGroup
	std::string ResetAll();

	//!@brief Set/unset 'boldface' format (rendered as 'high colour intensity' in some terminals)  
	//!@param active Toggles the command between activating the style (true) and deactivating (False)
	//!@returns The relevant ANSI sequence 
	Command Bold(bool active = true);
	
	//!@brief Set/unset 'low intensity' format
	//!@param active Toggles the command between activating the style (true) and deactivating (False)
	//!@returns The relevant ANSI sequence 
	Command Faint(bool active = true);

	//!@brief Set/unset Italic typeface 
	//!@param active Toggles the command between activating the style (true) and deactivating (False)
	//!@returns The relevant ANSI sequence 
	Command Italics(bool active = true);

	//!@brief Set/unset Underline format
	//!@param active Toggles the command between activating the style (true) and deactivating (False)
	//!@returns The relevant ANSI sequence 
	Command Underline(bool active = true);
	
	//!@brief Set/unset Highlight format
	//!@param active Toggles the command between activating the style (true) and deactivating (False)
	//!@returns The relevant ANSI sequence 
	Command Highlight(bool active = true);

	//!@brief Set/unset Strikethrough format
	//!@param active Toggles the command between activating the style (true) and deactivating (False)
	//!@returns The relevant ANSI sequence 
	Command Strike(bool active = true);

	//!@brief Sets colour of foreground(background) if input is false(true). 
	Command Black(bool targetBackgroundCol = false);

	//!@brief Sets colour of foreground(background) if input is false(true). 
	Command Blue(bool targetBackgroundCol = false);
	
	//!@brief Sets colour of foreground(background) if input is false(true). 
	Command Cyan(bool targetBackgroundCol = false);
	
	//!@brief Sets colour of foreground(background) if input is false(true). 
	Command Green(bool targetBackgroundCol = false);
	
	//!@brief Sets colour of foreground(background) if input is false(true). 
	Command Purple(bool targetBackgroundCol = false);
	
	//!@brief Sets colour of foreground(background) if input is false(true). 
	Command Red(bool targetBackgroundCol = false);
	
	//!@brief Sets colour of foreground(background) if input is false(true). 
	Command Yellow(bool targetBackgroundCol = false);
	
	//!@brief Sets colour of foreground(background) if input is false(true). 
	Command White(bool targetBackgroundCol = false);

	//!@brief Set subsequent text to the *default terminal* colour. 
	//!@param targetBackgroundCol if false (the default), the colour is used as the 'foreground' (text) colour. If true, the colour is assigned to the background. 
	//!@returns The associated ANSI command
	Command DefaultColour(bool targetBackgroundCol = false);
	
	//!@brief Sets the subsequent text to a colour specified by an 8-bit RGB colour code
	//!@param r A [0,255] value indicating the *red* intensity
	//!@param b A [0,255] value indicating the *blue* intensity
	//!@param g A [0,255] value indicating the *green* intensity
	//!@param targetBackgroundCol if false (the default), the colour is used as the 'foreground' (text) colour. If true, the colour is assigned to the background. 
	//!@warning Terminals often try to ensure text is visible and may reject some combinations of foreground/background colours based on their own internal contrast measurements. This is implementation specific - but usually means it is impossible to make text that cannot be read. 
	inline Command Colour(uint8_t r,uint8_t g, uint8_t b,bool targetBackgroundCol = false)
	{
		return Command(r,g,b,targetBackgroundCol ? Element::Background : Element::Foreground);
	}

	
}
	

 