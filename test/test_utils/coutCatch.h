#pragma once
#include <functional>
#include <iostream>
#include <sstream>
#include <string>

// Helper function to capture stdout
std::string capture_stdout(std::function<void()> func)
{
	std::stringstream ss;
	std::streambuf *old_buf = std::cout.rdbuf();

	struct RdbufGuard
	{
		std::ostream &stream;
		std::streambuf *original;
		~RdbufGuard() { stream.rdbuf(original); }
	} guard{std::cout, old_buf};

	std::cout.rdbuf(ss.rdbuf());
	func();
	std::cout.flush();
	return ss.str();
} // guard's destructor restores rdbuf here, even if func() threw
