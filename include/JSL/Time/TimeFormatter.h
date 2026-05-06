#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <string_view>
#include <chrono>

namespace JSL::Time
{
    /*!
		Given a duration in seconds, convert it into standard Day/Hour/Minute/Second formatted string. Times are restricted to a one-unit contrast (i.e. day + hour, hour + minute), with the final element being reported to 1 decimal accuracy
		\param timeInSeconds The time to be converted
		\returns A human-readable string equal to the input
	 */
    std::string FormatDuration(double timeInSeconds);

    std::string FormatDuration(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end);

    
}