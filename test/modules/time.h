#pragma once
#include <catch2/catch_test_macros.hpp>
#include "../../modules/Time/Time.h"
#include <thread>

using JSL::Timer;
TEST_CASE("Timer State Handling","[time][timer]")
{
	Timer T;

	SECTION("Test ordering")
	{
		REQUIRE_THROWS(T.Measure());
		REQUIRE_THROWS(T.Stop());
		T.Start();
		REQUIRE_NOTHROW(T.Measure());
		REQUIRE_NOTHROW(T.Stop());

	}

	SECTION("Sequential calls")
	{
		T.Start();
		REQUIRE_NOTHROW(T.Start());
		double firstCall = T.Measure();
		REQUIRE_NOTHROW(T.Stop());
		double secondCall = T.Measure();
		REQUIRE_NOTHROW(T.Measure());
		double pausedCall = T.Measure();
		REQUIRE(secondCall >= firstCall);
		REQUIRE(secondCall == pausedCall);
	}
	SECTION("Bounding the timer")
	{
		auto t = {1,50,100,300};
		for (int wait : t)
		{
			T.Start();
			std::this_thread::sleep_for(std::chrono::milliseconds(wait)); // Sleep for 50ms
			T.Stop();
			REQUIRE(T.Measure() >= 0.001*wait);
		}
	}

    
}

TEST_CASE("FormatDuration basic formatting", "[time][string]")
{
    SECTION("Early return for near-zero values")
    {
        REQUIRE(JSL::FormatDuration(0.00004) == "[Time less than 0.05ms]");
        REQUIRE(JSL::FormatDuration(0.0) == "[Time less than 0.05ms]");
    }

    SECTION("Negative values")
    {
        REQUIRE(JSL::FormatDuration(-60.0) == "Negative 1 Minute");
        REQUIRE(JSL::FormatDuration(-1.5) == "Negative 1.5 Seconds");
    }

    SECTION("Pluralization and Singular units")
    {
        REQUIRE(JSL::FormatDuration(1.0) == "1 Second");
        REQUIRE(JSL::FormatDuration(2.0) == "2 Seconds");
        REQUIRE(JSL::FormatDuration(60.0) == "1 Minute");
        REQUIRE(JSL::FormatDuration(120.0) == "2 Minutes");
    }
}

TEST_CASE("FormatDuration contrast and rounding", "[time][string]")
{
    SECTION("One-unit contrast logic")
    {
        // 90061s = 1 Day, 1 Hour, 1 Minute, 1 Second
        // Should only show Day + Hour
        REQUIRE(JSL::FormatDuration(90061.0) == "1 Day 1 Hour");
        
        // 3661s = 1 Hour, 1 Minute, 1 Second
        // Should only show Hour + Minute
        REQUIRE(JSL::FormatDuration(3661.0) == "1 Hour 1 Minute");
    }

    SECTION("Decimal tail and carry-over rounding")
    {
        // Check standard decimal
        REQUIRE(JSL::FormatDuration(61.5) == "1 Minute 1.5 Seconds");

        // Check the 0.96 -> 1.0 carry over logic
        // 0.96 rounds nonIntegerTail to 10, which should increment integerTail
        REQUIRE(JSL::FormatDuration(0.96) == "1 Second");
    }

    SECTION("High resolution threshold (< 0.1s)")
    {
        // Below 0.1s, the code should switch to Milliseconds
        REQUIRE(JSL::FormatDuration(0.05) == "50 Milliseconds");
        REQUIRE(JSL::FormatDuration(0.0055) == "5.5 Milliseconds");
    }
}

TEST_CASE("FormatDuration edge cases", "[time][string]")
{
    SECTION("Trailing space removal")
    {
        // Exact units should not have a trailing space
        std::string result = JSL::FormatDuration(3600.0);
        REQUIRE(result == "1 Hour");
        REQUIRE(result.back() != ' ');
    }

    SECTION("Large durations")
    {
        double oneYearOneDay = (86400.0 * 365.0) + 86400.0;
        REQUIRE(JSL::FormatDuration(oneYearOneDay) == "1 Year 1 Day");
    }

}