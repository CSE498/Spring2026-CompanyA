/**
 * @file Timer.cpp
 * @author Nathan Charters
 */

#include "Timer.hpp"

/**
 * Constructor for a Timer.
 */
Timer::Timer()
{
}

/**
 * Starts a timer, adding it to the map of timers using the provided
 * timer name as the key, and the current time as the value.
 */
void Timer::start(std::string timerName)
{
    mTimers[timerName] = std::chrono::steady_clock::now();
}

/**
 * Stops the timer with the provided name, returning the calculated time interval
 * as a duration in seconds. If the timer name is not found, returns a duration of 0 seconds.
 */
std::chrono::duration<double> Timer::stop(std::string timerName)
{
    if (mTimers.find(timerName) != mTimers.end())
    {
        auto startTime = mTimers[timerName];
        auto endTime = std::chrono::steady_clock::now();
        return endTime - startTime;
    }
    else
    {
        return std::chrono::duration<double>(0);
    }
}