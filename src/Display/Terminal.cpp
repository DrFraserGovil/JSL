#include <JSL/Display/Terminal.h>

#include <string>
#if defined(_WIN32) || defined(_WIN64)
#include <io.h>
#include <windows.h>
#else
#include <sys/ioctl.h>
#include <termios.h>
#include <unistd.h>
#endif
using namespace std::string_literals;
namespace JSL::Display
{
	TerminalCommand MoveToColumn(uint32_t column)
	{
		column = column > 0 ? column : 1; // ensure that 0-indexing doesn't cause issues as columns are 1 indexed
		return "\033["s + std::to_string(column) + "G"s;
	}

	TerminalCommand Move(Direction dir, unsigned int steps)
	{
		return "\033["s + std::to_string(steps) + (char)(dir + 65);
	}

	GlobalEnvironment::GlobalEnvironment()
	{
		CacheANSI(); // do this first because it is needed elsewhere
		CacheSize();
	}
	GlobalEnvironment &Terminal()
	{
		static GlobalEnvironment instance;
		return instance;
	}

	void GlobalEnvironment::CacheANSI()
	{
		// have to do some preprocessor messiness to get the right function as it's platform dependent
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
		return RowCount;
	}
	size_t GlobalEnvironment::Columns()
	{
		if (DynamicUpdates)
			CacheSize();
		return ColumnCount;
	}
	size_t GlobalEnvironment::TabSize()
	{
		return UserTabs;
	}

	void GlobalEnvironment::SetTabSize(size_t newtabs)
	{
		UserTabs = newtabs;
	}
	void GlobalEnvironment::CacheSize()
	{
		// set defaults
		RowCount = 24;
		ColumnCount = 80;

#if defined(_WIN32) || defined(_WIN64)
		HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
		if (hOut == INVALID_HANDLE_VALUE || !AnsiActive)
		{
			return;
		}

		CONSOLE_SCREEN_BUFFER_INFO csbi;
		if (!GetConsoleScreenBufferInfo(hOut, &csbi))
		{
			return;
		}

		// srWindow specifies the window coordinates relative to the console buffer.
		// Width and height are inclusive, so we add 1 to the differences.
		RowCount = static_cast<size_t>(csbi.srWindow.Right - csbi.srWindow.Left + 1);
		ColumnCount = static_cast<size_t>(csbi.srWindow.Bottom - csbi.srWindow.Top + 1);
#else
		struct winsize ws;
		if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1)
		{
			// Fallback or error handling if not a TTY
			return;
		}
		RowCount = ws.ws_row;
		ColumnCount = ws.ws_col;
#endif
	}
} // namespace JSL::Display
