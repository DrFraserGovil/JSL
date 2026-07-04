#pragma once
#include <cmath>
#include <cstdint>
#include <string>

namespace JSL::Display
{
	//!@brief A basic wrapper to make TerminalCommands look more fancy than simple strings, and to remind users that the result of treating them as strings is non-trivial (i.e. the length of these strings does not equate to their length on screen!)
	typedef std::string TerminalCommand;

	//! @brief Hides the cursor
	const TerminalCommand HideCursor = "\033[?25l";

	//! @brief Shows the cursor
	const TerminalCommand ShowCursor = "\033[?25h";

	//! @brief Moves the cursor up one line
	const TerminalCommand CursorUp = "\033[A";

	//! @brief Deletes the previous character (except newlines)
	//!@details \\b on its own just moves cursor; this moves cursor, erases character, then moves it back again
	const TerminalCommand Backspace = "\b \b";

	//! @brief Erases all characters to the right of the cursor position
	const TerminalCommand EraseAllRight = "\033[0K";

	//! @brief Erases all characters to the left of the cursor position
	const TerminalCommand EraseAllLeft = "\033[1K";

	//! @brief Clears the entire line (but does not move the cursor position)
	const TerminalCommand ClearLine = "\033[2K\r";

	//! @brief Clears the entire terminal (and scrollback buffer)
	const TerminalCommand ClearScreen = "\033[3J\r";

	//! @brief Moves the cursor to position (1,1) (the top left of the screen)
	const TerminalCommand ResetPosition = "\033[1;1\r";

	//! @brief Specify the direction of terminal movement
	//! @details The fixed numbers allow us to do some ASCII hackery, and convert the enums to relevant chars
	enum Direction
	{
		Up = 0,
		Down = 1,
		Right = 2,
		Left = 3
	};

	//! @brief Move the cursor a specified number of steps in a given direction
	TerminalCommand Move(Direction dir, unsigned int steps);

	//! @brief Moves the cursor to the specified column in the current line (no other text is affected)
	//! @brief Columns are 1-indexed
	TerminalCommand MoveToColumn(uint32_t column);

	/*!
	 * @brief A class representing the current state of the terminal
	 */
	class GlobalEnvironment
	{
	  public:
		//! @brief If true, Rows() and Columns() auto-update every time they are queried. If false, they update only on construction and when Cache() calls are made
		//! @details The TabSize and the ANSI-Code status are *not* queried as the method of querying them is more involved, and it would be unusual for them to change during runtime (whilst it is expected a user may resize their terminal window)
		bool DynamicUpdates = false;
		//! The number of vertical lines in the current terminal (without scrolling)
		size_t Rows();
		//! The number of horizontal characters per line in the current terminal
		size_t Columns();
		//! The size of a `tab block' in the current terminal
		size_t TabSize();
		//! @brief Computes the size of a tabstop and caches it for future use
		//! @details Functions by inserting a tab, requesting a Device Status Report to find the current column, then deleting the line. This makes it somewhat fragile and expensive; so it is recommended that this is not queried often (it runs on startup).
		void CacheTabs();

		//! Uses ioctl to query the current dimensions of the terminal, and cache them for future retrieval
		void CacheSize();

		/*! @brief  Determine if the output stream can interpret ANSI escape sequences
			@details Uses isatty (and _isatty for windows) to determine if stdout can render ANSI commands.
			Caches the result in AnsiActive: True if the output is capable of interpreting format commands (i.e. a terminal)
			False if the terminal is plaintext (i.e. a file)
		*/
		void CacheANSI();

		/*!
		 * @brief Query the cached value to determine if the output stream can interpret ANSI escape sequences
		 * @return  True if the output is capable of interpreting format commands (i.e. a terminal) False if the terminal is plaintext (i.e. a file)
		 */
		bool IsANSICapable();
		// Declared friend so singleton can construct
		friend GlobalEnvironment &Terminal();

	  private:
		//! Private constructor for singleton uniqueness
		GlobalEnvironment();
		//! Internal row cache
		size_t _Rows;
		//! Internal column cache
		size_t _Columns;
		//! Internal tab cache
		size_t _Tabsize;

		//! ansi state
		bool AnsiActive;

		//! Default destructor; only here for Ro5 purposes really!
		~GlobalEnvironment() = default;
		//! Delete to ensure singleton uniqueness
		GlobalEnvironment(const GlobalEnvironment &) = delete;
		//! Delete to ensure singleton uniqueness
		GlobalEnvironment &operator=(const GlobalEnvironment &) = delete;
		//! Delete to ensure singleton uniqueness
		GlobalEnvironment(GlobalEnvironment &&) = delete;
		//! Delete to ensure singleton uniqueness
		GlobalEnvironment &operator=(GlobalEnvironment &&) = delete;
	};

	/*!
	 * @brief Access the global terminal environment state
	 * @return The singleton-instance of the GlobalEnvironment object
	 */
	GlobalEnvironment &Terminal();
}; // namespace JSL::Display
