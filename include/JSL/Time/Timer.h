#pragma once
#include <chrono>

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
            Timer();
            
            void Start();
            
            void Stop();
            
            double Lap();
            
            double Measure();

            std::string Display();
    };
}