#pragma once
#include <string>
#include <vector>
#include <sstream>
#include <string_view>
#include <chrono>
namespace JSL
{
    /*!
		Given a duration in seconds, convert it into standard Day/Hour/Minute/Second formatted string. Times are restricted to a one-unit contrast (i.e. day + hour, hour + minute), with the final element being reported to 1 decimal accuracy
		\param timeInSeconds The time to be converted
		\returns A human-readable string equal to the input
	 */

    namespace
    {
        constexpr std::string_view divisions[] = {"Year","Day", "Hour", "Minute", "Second","Millisecond"};
        constexpr double duration[] = {86400*365,86400, 3600, 60, 1,1e-3};
    }
	inline std::string FormatDuration(double timeInSeconds)
	{
		
        std::ostringstream output;
		if (timeInSeconds < 0)
		{
			timeInSeconds = std::abs(timeInSeconds);
			output << "Negative ";
		}
        int originalContrast =  4;//divisions.size() -1;
        if (timeInSeconds < 0.1) 
        {
            if (timeInSeconds < 5e-5)
            {
                return "[Time less than 0.05ms]";
            }
            originalContrast = 5;
        }

        int timeContrast =originalContrast;
        bool trailingSpace = false;
		for (int i = 0; i < timeContrast; ++i)
		{

			int v = static_cast<int>(timeInSeconds / duration[i]);
			timeInSeconds -= v * duration[i];
			
			if (v > 0)
			{
                if (timeContrast == originalContrast) //ensure we don't have 1 day 0.01 seconds; keep the contrast 
                {
                    timeContrast = i + 1;
                }
				output << v << " " << divisions[i];
				if (v > 1)
				{
					output << "s";
				}
				output << " ";
                trailingSpace = true;
			}
		}
        //now deal with the hanging tail
        int integerTail = static_cast<int>(timeInSeconds/duration[timeContrast]);
        timeInSeconds -= integerTail * duration[timeContrast];
        int nonIntegerTail = static_cast<int>((10*timeInSeconds)/duration[timeContrast]+0.5);
        if (nonIntegerTail == 10)
        {
            ++integerTail;
            nonIntegerTail = 0;
        }
        if (integerTail > 0 || nonIntegerTail > 0)
        {
            output << integerTail;
            if (nonIntegerTail > 0)
            {
                output << "." << nonIntegerTail;
            }
            output << " " << divisions[timeContrast];
            if (integerTail != 1 || nonIntegerTail > 0)
            {
                output << "s";
            }
        }
        else if (trailingSpace)
        {
            std::string tmp = output.str();
            tmp.pop_back();
            return tmp;
        }

		return output.str();
	}

    inline std::string FormatDuration(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end)
	{
		std::chrono::duration<double> diff = end - start;
		double seconds = diff.count();
		
		
		return FormatDuration(seconds);
	}

    
}