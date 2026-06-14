#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <string_view>
#include <chrono>

namespace JSL::Time
{

    /// @brief  Convert a duration in seconds into a formatted string
	/// @details The time is split into the largest unit of Day/Hour/Minute/Second and one unit smaller (i.e. day + hour, hour + minute), with the second element reported to 1 decimal accuracy.  
    /// @param timeInSeconds The time to be converted
    /// @return A human readable string representing the input
    std::string FormatDuration(double timeInSeconds);

    /// @brief  Convert the difference between two time points into a formatted string
	/// @details The time is split into the largest unit of Day/Hour/Minute/Second and one unit smaller (i.e. day + hour, hour + minute), with the second element reported to 1 decimal accuracy.  
    /// @param start The beginning of the time period to be measured
    /// @param end The end of the time period to be measured
    /// @return A human readable string representing the input
    std::string FormatDuration(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end);

    
}