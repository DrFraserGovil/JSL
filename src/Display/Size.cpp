#include <JSL/Display/Size.h>

#include <sys/ioctl.h>
#include <unistd.h>
#include <iostream>
JSL::Terminal::Size JSL::Terminal::GetDimensions()
{
	struct winsize ws;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == -1) 
	{
		// Fallback or error handling if not a TTY
		return { 80, 24 };
	}
	return { ws.ws_col, ws.ws_row };
}
