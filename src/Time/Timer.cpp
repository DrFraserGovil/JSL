#include <JSL/Time/Timer.h>
#include <JSL/Time/TimeFormatter.h>
#include <chrono>
#include <exception>
namespace JSL::Time
{


    Timer::Timer()
    {
        startCalled = false;
        stopCalled = false;
    }
    void Timer::Start()
    {
        start_time = std::chrono::high_resolution_clock::now();
        startCalled = true;
        stopCalled = false;
    }
    void Timer::Stop()
    {
        if (!startCalled)
        {
            throw std::logic_error("Cannot stop the timer without first calling Timer.start()");
        }
        stopCalled = true;
        stop_time = std::chrono::high_resolution_clock::now();
    }

    double Timer::Lap()
    {
        if (!startCalled)
        {
            throw std::logic_error("Cannot measure a time without first calling Timer.start()");
        }
        using namespace std::chrono;
        return duration_cast<nanoseconds>
            (std::chrono::high_resolution_clock::now() - start_time).count() / 1e9;
    }

    double Timer::Measure()
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
    std::string Timer::Display()
    {
        return FormatDuration(Measure());
    }
}