#pragma once
#include <cstdint>
#include <JSL/Display/Format.h>
/*
	These are some display commands which  enable the user to shift the cursor around, delete items and otherwise alter the appearance of terminal output. 

*/


namespace JSL::Display
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
	

 