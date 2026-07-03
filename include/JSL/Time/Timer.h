#pragma once
#include <chrono>

namespace JSL::Time
{
	/// @brief A class for measuring the duration between two function calls
	class Timer
	{
	  private:
		/// @brief The start period of the timer, assigned during Start()
		std::chrono::high_resolution_clock::time_point start_time;

		/// @brief The end period of the timer, assigned during Stop() or Measure()
		std::chrono::high_resolution_clock::time_point stop_time;

		/// @brief True if the start_time has been assigned to
		bool startCalled;

		/// @brief True if a stop_time has been assigned. Set to false by each Start() call.
		bool stopCalled;

	  public:
		/// @brief Initialises a new Timer obejct in a default state (awaiting a Start() command)
		Timer();

		/// @brief Record the current time as `start', and mark the timer as running
		/// @details Subsequent calls to Start() override this time (and delete any previously recorded 'end' times)
		void Start();

		/// @brief Records the current time as `end'
		/// @brief Does not alter the state of the stopwatch; multiple Stop() calls can be made, each overwriting the previous end time.
		/// @throws runtime_error if called before whilst the timer is not running
		void Stop();

		/// @brief Return the time (in seconds) between the stop time and the start time.
		/// @details If Stop() was not called first, it is called here
		/// @return The seconds measured by the timer
		/// @throws runtime_error if called before a start time is assigned
		double Measure();

		/// @brief Peek at the current value of the timer, without stopping it
		/// @return THe seconds between the moment Lap() is called and the start time
		/// @throws runtime_error if called before a start time is assigned
		double Lap();

		/// @brief Equal to FormatDuration(Measure())
		/// @returns A human-readable string representing the elapsed time
		std::string Display();
	};
} // namespace JSL::Time
