#pragma once
#include <chrono>

#include <exception>
#include "TimeFormatter.h"
namespace JSL
{
    class Timer
    {
        private:
            std::chrono::high_resolution_clock::time_point start_time;
            std::chrono::high_resolution_clock::time_point stop_time;
            bool startCalled;
            bool stopCalled;

        public:
            Timer()
            {
                startCalled = false;
                stopCalled = false;
            }
            void Start()
            {
                start_time = std::chrono::high_resolution_clock::now();
                startCalled = true;
                stopCalled = false;
            }

            void Stop()
            {
                if (!startCalled)
                {
                    throw std::logic_error("Cannot stop the timer without first calling Timer.start()");
                }
                stopCalled = true;
                stop_time = std::chrono::high_resolution_clock::now();
            }


            double Measure()
            {
                if (!startCalled)
                {
                    throw std::logic_error("Cannot measure a time without first calling Timer.start()");
                }
                if (!stopCalled)
                {
                    Stop();
                }
                using namespace std::chrono;
                return duration_cast<nanoseconds>
                    (stop_time - start_time).count() / 1e9;
            }
            std::string Display()
            {
                return FormatDuration(Measure());
            }
    };
}