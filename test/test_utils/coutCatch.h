#pragma once
#include <string>
#include <functional>
#include <sstream>
#include <iostream>


// Helper function to capture stdout
std::string capture_stdout(std::function<void()> func)
{
    std::stringstream ss;
    // Save the original buffer
    std::streambuf* old_buf = std::cerr.rdbuf();
    // Redirect cout to the stringstream buffer
    std::cerr.rdbuf(ss.rdbuf());
    // Call the function that produces output
    func();
    // Restore the original buffer
    std::cerr.rdbuf(old_buf);
    // Return the captured string
    return ss.str();
}




