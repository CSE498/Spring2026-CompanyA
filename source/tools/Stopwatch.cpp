/**
 * @file Stopwatch.cpp
 * @author Nathan Charters
 */

#include "Stopwatch.hpp"

// The elapsed time is calculated internally in nanoseconds.
// This variable allows for simple conversion in methods that return a value in seconds.
constexpr double NS_TO_S_CONVERSION = 1000000000.0;

/**
 * Stopwatch constructor.
 * @param name The name of the Stopwatch.
 * @param startRunning Whether the Stopwatch should start running. Defaults to true.
 */
cse498::Stopwatch::Stopwatch(std::string name, bool startRunning)
{
    mName = name;
    mRunning = startRunning;
    mStart = std::chrono::steady_clock::now();
    mLapStart = std::chrono::steady_clock::now();
}

/**
 * Start the Stopwatch.
 * If the Stopwatch is already running, it won't do anything.
 */
void cse498::Stopwatch::start()
{
    if (!mRunning)
    {
        mRunning = true;
        mStart = std::chrono::steady_clock::now();
        mLapStart = std::chrono::steady_clock::now();
    }
}

/**
 * Stop the Stopwatch and update the elapsed time and current lap time.
 * If the Stopwatch is already stopped, it won't do anything.
 */
void cse498::Stopwatch::stop()
{
    if (mRunning)
    {
        mRunning = false;
        mElapsed += (std::chrono::steady_clock::now() - mStart);
        mLaps.at(mLaps.size() - 1) += (std::chrono::steady_clock::now() - mLapStart).count() / NS_TO_S_CONVERSION;
    }
}

/**
 * Reset the Stopwatch, turning it off, setting its elapsed time back to 0, and resetting the laps.
 */
void cse498::Stopwatch::reset()
{
    mRunning = false;
    mElapsed = std::chrono::duration<double>::zero();
    mLaps.clear();
    mLaps.push_back(0.0);
}

/**
 * Restart the Stopwatch, resetting its elapsed time back to 0 and resetting the laps,
 * but starting it up immediately.
 */
void cse498::Stopwatch::restart()
{
    mElapsed = std::chrono::duration<double>::zero();
    mLaps.clear();
    mLaps.push_back(0.0);
    mRunning = true;
    mStart = std::chrono::steady_clock::now();
    mLapStart = std::chrono::steady_clock::now();
}

/**
 * Lap the Stopwatch, stopping the previous lap and immediately starting a new one.
 */
void cse498::Stopwatch::lap()
{
    mLaps.at(mLaps.size() - 1) += (std::chrono::steady_clock::now() - mLapStart).count() / NS_TO_S_CONVERSION;
    mLaps.push_back(0.0);
    mLapStart = std::chrono::steady_clock::now();
}

/**
 * Gets and calculates the total time (in seconds) that the Stopwatch has run for.
 * @return Total run time of Stopwatch (seconds).
 */
double cse498::Stopwatch::elapsed() const
{
    if (mRunning)
    {
        // Subtracts the starting time point from the current time and adds that duration to any already accumulated duration.
        return (mElapsed + (std::chrono::steady_clock::now() - mStart)).count() / NS_TO_S_CONVERSION;
    }
    return mElapsed.count() / NS_TO_S_CONVERSION;
}

/**
 * Checks whether the Stopwatch is currently running.
 * @return Whether the Stopwatch is running or not.
 */
bool cse498::Stopwatch::isRunning() const
{
    return mRunning;
}

/**
 * Returns the vector containing the lap times for the Stopwatch.
 * If the Stopwatch is running, add the elapsed time into the current lap time to get an accurate lap time,
 * and adjust the starting point as necessary for future calculations.
 */
std::vector<double> cse498::Stopwatch::getLaps()
{
    if (mRunning)
    {
        mLaps.at(mLaps.size() - 1) += (std::chrono::steady_clock::now() - mLapStart).count() / NS_TO_S_CONVERSION;
        mLapStart = std::chrono::steady_clock::now();
    }
    return mLaps;
}

/**
 * Mock an advance of time by the given interval (in seconds).
 * This method should only be used for testing purposes, and has no use outside of making sure elapsed() works properly.
 * @param seconds The amount of time to advance by.
 */
void cse498::Stopwatch::advanceTime(double seconds)
{
    if (mRunning)
    {
        mElapsed += std::chrono::duration<double>(seconds);
        mLaps.at(mLaps.size() - 1) += std::chrono::duration<double>(seconds).count();
    }
}