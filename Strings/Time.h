#pragma once
#include <chrono>
#include <string>
#include <sstream>
#include <vector>
#include <iostream>


namespace JSL
{
	
	/*!
		Get current system time in a readable format
	 * \returns A string of the readable format   
	*/
	inline std::string PrintCurrentTime()
	{
		auto now = std::chrono::system_clock::now();
		std::time_t now_t = std::chrono::system_clock::to_time_t(now);
		std::ostringstream out;
		out << "Current time is: ";
		out << std::ctime(&now_t);
		return out.str();
	}
	
	/*!
		Given a duration in seconds, convert it into standard Day/Hour/Minute/Second formatted string. 
		\param timeInSeconds The time to be converted
		\returns A human-readable string equal to the input
	 */
	inline std::string FormatDuration(int timeInSeconds)
	{
		std::vector<std::string> divisions= {"Day", "Hour", "Minute", "Second"};
		std::vector<int> duration = {86400, 3600, 60 , 1};
		//std::vector<int> chunks = std::vector(0,divisions.size());
		
		bool nothingAdded = true;
		std::string output = "";
		for (int i = 0; i < divisions.size(); ++i)
		{
			int v = timeInSeconds / duration[i];
			timeInSeconds -= v * duration[i];
			
			if (v > 0)
			{
				nothingAdded = false;
				output += std::to_string(v) + " " + divisions[i];
				if (v > 1)
				{
					output += "s";
				}
				output += " ";
			}
		}
		if (nothingAdded)
		{
			output = "less than 1 second";
		}	
	}
	
	/*!
	 Calls FormatDuration() on the duration of the start and endpoints of a ``chrono`` stopwatch. 
	 * \param start A ``chrono`` object representing the start of the duration
	 * \param end The corresponding end-point ``chrono``
	*/
	inline std::string FormatClock(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end)
	{
		std::chrono::duration<double> diff = end - start;
		double seconds = diff.count();
		
		
		return FormatDuration(seconds);
	}

}
