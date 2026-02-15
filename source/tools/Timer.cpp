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
    if (mTimers.find(timerName) != mTimers.end())
    {
        if (!mTimers[timerName].isRunning)
        {
            // If the timer already exists and is stopped, start it back up and keep the accumulated duration.
            mTimers[timerName].startTime = std::chrono::steady_clock::now();
            mTimers[timerName].isRunning = true;
        }
    }
    else
    {
        // If the timer doesn't exist, create it and start it.
        mTimers[timerName].startTime = std::chrono::steady_clock::now();
        mTimers[timerName].isRunning = true;
        mTimers[timerName].accumulatedDuration = std::chrono::duration<double>::zero();
    }
}

/**
 * Stops the timer with the provided name, addings it's elapsed time to the
 * accumulated duration and marking it as stopped. Does nothing if the timer doesn't exist.
 */
void Timer::stop(std::string timerName)
{
    if (mTimers.find(timerName) != mTimers.end())
    {
        // If the timer exists, calculate how long it ran for, add that to the accumulated duration,
        // and mark it as stopped.
        auto startTime = mTimers[timerName].startTime;
        auto endTime = std::chrono::steady_clock::now();
        mTimers[timerName].accumulatedDuration += endTime - startTime;
        mTimers[timerName].isRunning = false;
    }
}

/**
 * Restarts the timer with the provided name, resetting its accumulated duration to 0 and starting it again.
 */
void Timer::restart(std::string timerName)
{
    mTimers[timerName].startTime = std::chrono::steady_clock::now();
    mTimers[timerName].isRunning = true;
    mTimers[timerName].accumulatedDuration = std::chrono::duration<double, std::nano>::zero();
}

/**
 * Get the duration the timer has run for, adding in the current running time if the timer is still running.
 * Returns a duration of 0 if the timer doesn't exist.
 */
double Timer::getTime(std::string timerName)
{
    if (mTimers.find(timerName) != mTimers.end())
    {
        if (mTimers.at(timerName).isRunning)
        {
            // If the timer exists, return the accumulated duration plus
            // the current running time if it's still running.
            auto startTime = mTimers.at(timerName).startTime;
            auto currentTime = std::chrono::steady_clock::now();
            auto elapsed = mTimers.at(timerName).accumulatedDuration + (currentTime - startTime);
            return elapsed.count() / 1000000000.0; // Convert from nanoseconds to seconds.
        }
        else
        {
            return mTimers.at(timerName).accumulatedDuration.count() / 1000000000.0; // Convert from nanoseconds to seconds.
        }
    }
    else
    {
        // If the timer doesn't exist, return a duration of 0.
        return 0.0;
    }
}