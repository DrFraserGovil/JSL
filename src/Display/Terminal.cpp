#include <JSL/Display/Terminal.h>

#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
#include <termios.h>
#if defined(_WIN32) || defined(_WIN64)
	#include <io.h>
#else
	#include <unistd.h>
#endif

namespace JSL::Display::Terminal
{
	Environment::Environment()
	{
        CacheANSI(); // do this first because it is needed elsewhere
		CacheSize();
		CacheTabs();
	}
	Environment & Global()
	{
		static Environment instance;	
		return instance;
	}

    void Environment::CacheANSI()
    {
        //have to do some preprocessor messiness to get the right function as it's platform dependent
		 #ifdef _WIN32
			AnsiActive = _isatty(_fileno(stdout));
		#else
			AnsiActive = isatty(fileno(stdout));
		#endif
    }
    bool Environment::IsANSICapable()
    {
        return AnsiActive;
        // return true;
    }
	size_t Environment::Rows()
	{
		return _Rows;
	}
	size_t Environment::Columns()
	{
		if (DynamicUpdates) CacheSize();
		return _Columns;
	}
	size_t Environment::TabSize()
	{
		if (DynamicUpdates) CacheTabs();
		return _Tabsize;
	}
	void Environment::CacheTabs()
	{
		size_t dtab = 8;
		if (!AnsiActive)
		{
			_Tabsize = dtab;
			return;
		}
        struct termios oldt;
        if (tcgetattr(STDIN_FILENO, &oldt) < 0)
        {
			_Tabsize = dtab;
            return; // Safe fallback if stdin isn't a valid terminal
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
			_Tabsize = dtab;
            return;
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
			_Tabsize = dtab;
            return; // Timeout or corrupt stream read occurred, escape with fallback
        }

        // Isolate the column characters located between ';' and 'R'
        std::string colStr = response.substr(semi + 1, response.size() - semi - 2);
		
        try
        {
            int col = std::stoi(colStr);
            _Tabsize = static_cast<size_t>(col - 1); // 1-indexed to 0-indexed column difference
        }
        catch (...)
        {
            _Tabsize = dtab; 
        }
		return;
	}
	void Environment::CacheSize()
	{
		struct winsize ws;
		if (!AnsiActive || ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) 
		{
			// Fallback or error handling if not a TTY
			_Rows = 24;
			_Columns = 80;
			return;
		}
		_Rows = ws.ws_row;
		_Columns = ws.ws_col;
	}	
}