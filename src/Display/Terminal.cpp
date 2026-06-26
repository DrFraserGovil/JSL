#include <JSL/Display/Terminal.h>

#include <iostream>
#include <string>
#if defined(_WIN32) || defined(_WIN64)
	#include <io.h>
	#include <windows.h>
#else
	#include <unistd.h>
	#include <sys/ioctl.h>
	#include <termios.h>
#endif
using namespace std::string_literals;
namespace JSL::Display
{
	TerminalCommand MoveToColumn(uint32_t column)
	{
		column = column > 0 ? column : 1; //ensure that 0-indexing doesn't cause issues as columns are 1 indexed
		return "\033["s + std::to_string(column) + "G"s;
	}
	
	TerminalCommand Move(Direction dir, unsigned int steps)
	{
		return "\033["s + std::to_string(steps) + (char)(dir+65);
	}

	GlobalEnvironment::GlobalEnvironment()
	{
        CacheANSI(); // do this first because it is needed elsewhere
		CacheSize();
		CacheTabs();
	}
	GlobalEnvironment & Terminal()
	{
		static GlobalEnvironment instance;	
		return instance;
	}

    void GlobalEnvironment::CacheANSI()
    {
        //have to do some preprocessor messiness to get the right function as it's platform dependent
		 #ifdef _WIN32
			AnsiActive = _isatty(_fileno(stdout));
		#else
			AnsiActive = isatty(fileno(stdout));
		#endif
    }
    bool GlobalEnvironment::IsANSICapable()
    {
        return AnsiActive;
        // return true;
    }
	size_t GlobalEnvironment::Rows()
	{
		return _Rows;
	}
	size_t GlobalEnvironment::Columns()
	{
		if (DynamicUpdates) CacheSize();
		return _Columns;
	}
	size_t GlobalEnvironment::TabSize()
	{
		if (DynamicUpdates) CacheTabs();
		return _Tabsize;
	}

#if defined(_WIN32) || defined(_WIN64)
	size_t winTabs(size_t dtab)
	{
		// This was coded by Gemini, and I hate it
		// TODO: replace this with something not vibe coded
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE) return dtab;

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(hOut, &csbi)) return dtab;

		// In the Windows Console API, the current column is tracked in 
		// csbi.dwCursorPosition.X. 
		// When you write a '\t', the console advances to the next 8-character 
		// boundary by default.

		// We simulate your \r\t logic:
		// 1. Get current position
		// 2. Move to 0
		// 3. Print a tab
		// 4. Get new position

		COORD originalPos = csbi.dwCursorPosition;

		// Move to start of line
		COORD startOfLine = { 0, originalPos.Y };
		SetConsoleCursorPosition(hOut, startOfLine);

		// Write tab
		DWORD written;
		WriteConsoleA(hOut, "\t", 1, &written, NULL);

		// Get new position
		CONSOLE_SCREEN_BUFFER_INFO csbiNew;
		GetConsoleScreenBufferInfo(hOut, &csbiNew);
		size_t tabWidth = static_cast<size_t>(csbiNew.dwCursorPosition.X);

		// Reset to original
		SetConsoleCursorPosition(hOut, originalPos);

		return tabWidth;
	}
#else
	size_t nixTabs(size_t dtab)
	{
		struct termios oldt;
		if (tcgetattr(STDIN_FILENO, &oldt) < 0)
		{
			return dtab;
			// Safe fallback if stdin isn't a valid terminal
		}

		// 2. Configure raw mode variables
		struct termios raw = oldt;
		raw.c_lflag &= ~(ICANON | ECHO); // Turn off line-buffering and screen-echoing

		// Crucial Failsafe: Set a 500ms timeout (5 deciseconds) so non-compliant 
		// terminals or redirected pipes don't freeze the application process indefinitely.
		raw.c_cc[VMIN] = 0;
		raw.c_cc[VTIME] = 2;

		if (tcsetattr(STDIN_FILENO, TCSANOW, &raw) < 0)
		{
			return dtab;
		}

		// 3. Write the query string sequence to stdout
		// \r  = Move to column 1
		// \t  = Fire the tab character jump
		// \033[6n = Request cursor position report
		std::cout << "\r\t\033[6n" << std::flush;

		// 4. Capture the terminal response bytes from stdin
		// The expected sequence layout is: \033[row;colR
		std::string response;
		char ch;
		while (read(STDIN_FILENO, &ch, 1) > 0)
		{
			response += ch;
			if (ch == 'R')
			{
				break;
			}
		}

		// 5. Instantly clear the line to wipe our test tracking text out of sight
		// \033[2K vaporizes the entire active line contents
		std::cout << "\r\033[2K" << std::flush;

		// 6. Instantly restore the terminal back to its original configuration state
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

		// 7. Parse out the column index integer
		size_t semi = response.find(';');
		if (semi == std::string::npos || response.empty() || response.back() != 'R')
		{
			return dtab; // Timeout or corrupt stream read occurred, escape with fallback
		}

		// Isolate the column characters located between ';' and 'R'
		std::string colStr = response.substr(semi + 1, response.size() - semi - 2);

		try
		{
			int col = std::stoi(colStr);
			return static_cast<size_t>(col - 1); // 1-indexed to 0-indexed column difference
		}
		catch (...)
		{
			return dtab;
		}
	}
#endif
	void GlobalEnvironment::CacheTabs()
	{
		size_t dtab = 8;
		if (!AnsiActive)
		{
			_Tabsize = dtab;
			return;
		}

		#if defined(_WIN32) || defined(_WIN64)
		_Tabsize = winTabs(dtab);
#else
		_Tabsize = nixTabs(dtab);
#endif
		return;

	}
	void GlobalEnvironment::CacheSize()
	{
		//set defaults
		_Rows = 24;
		_Columns = 80;
		

		#if defined(_WIN32) || defined(_WIN64)
			HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
			if (hOut == INVALID_HANDLE_VALUE || !AnsiActive)
			{
				_Rows = 24;
				_Columns = 80;
				return;
			}

			CONSOLE_SCREEN_BUFFER_INFO csbi;
			if (!GetConsoleScreenBufferInfo(hOut, &csbi))
			{
				_Rows = 24;
				_Columns = 80;
				return;
			}

			// srWindow specifies the window coordinates relative to the console buffer.
			// Width and height are inclusive, so we add 1 to the differences.
			_Columns = static_cast<size_t>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
			_Rows = static_cast<size_t>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
		#else
			if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) 
			{
				// Fallback or error handling if not a TTY
				_Rows = 24;
				_Columns = 80;
				return;
			}
			struct winsize ws;
			_Rows = ws.ws_row;
			_Columns = ws.ws_col;
		#endif
		
	}	
}