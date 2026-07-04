.. _time:

Time
=========

The following commands provide a convenient interface to measure and represent temporal periods.

.. _time_format:
    
Formatting Commands
----------------------

.. jsl-meta::
	:file: Time/TimeFormatter.h
	:namespace: JSL::Time

.. doxygenfunction:: FormatDuration(double timeInSeconds)

.. doxygenfunction::  FormatDuration(std::chrono::time_point<std::chrono::system_clock> start, std::chrono::time_point<std::chrono::system_clock> end)

.. _stopwatch:
    
Stopwatch & Timer
-------------------

.. jsl-class:: JSL::Time::Timer
	:file: TIme/Timer.h
