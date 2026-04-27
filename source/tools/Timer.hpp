/**
 * @file Timer.hpp
 * @author Nathan Charters
 *
 * A class representing a Timer object.
 */

#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <cmath>
#include <iomanip>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace cse498 {
/**
 * A class representing a Timer object.
 *
 * A Timer keeps track of a single timing instance, and can be started, stopped,
 * reset, and restarted.
 */
class Timer {
public:
    /**
     * Timer constructor.
     * @param name The name of the Timer.
     * @param startRunning Whether the Timer should start running. Defaults to true.
     */
    Timer(std::string name, bool startRunning = true);

    /**
     * Start the Timer.
     * If the Timer is already running, it won't do anything.
     */
    void start();

    /**
     * Stop the Timer and update the elapsed time and current lap time.
     * If the Timer is already stopped, it won't do anything.
     */
    void stop();

    /**
     * Reset the Timer, turning it off, setting its elapsed time back to 0, and
     * resetting the laps.
     */
    void reset();

    /**
     * Restart the Timer, resetting its elapsed time back to 0 and resetting the
     * laps, but starting it up immediately.
     */
    void restart();

    /**
     * Lap the Timer, stopping the previous lap and immediately starting a new one.
     */
    void lap();

    /**
     * Gets and calculates the total time (in seconds) that the Timer has run for.
     * @return Total run time of Timer (seconds).
     */
    double elapsed() const;

    /**
     * Checks whether the Timer is currently running.
     * @return Whether the Timer is running or not.
     */
    bool isRunning() const;


    /**
     * If the Timer is running, add the elapsed time into the current lap time to
     * get an accurate lap time, and adjust the starting point as necessary for
     * future calculations.
     * @return Returns the vector containing the lap times for the Timer.
     */
    std::vector<double> getLaps();

    /**
     * Mock an advance of time by the given interval (in seconds).
     * This method should only be used for testing purposes, and has no use outside
     * of making sure elapsed() works properly.
     * @param seconds The amount of time to advance by.
     */
    void advanceTime(double seconds);

    /**
     * @param withLaps Should the string include lap information or not
     * @return Returns a string containing the state of the timer in xx:xx.xxx format
     * (minutes, seconds, milliseconds)
     */
    std::string toString(bool withLaps) const;

private:
    // The name of the Timer.
    std::string mName;

    // The starting time point of the Timer.
    std::chrono::time_point<std::chrono::steady_clock> mStart;

    // The total elapsed time of the Timer. Initialized to 0.
    std::chrono::duration<double, std::nano> mElapsed{std::chrono::duration<double>::zero()};

    // A boolean representing whether the Timer is running or not.
    bool mRunning;

    // The starting time point of the current lap.
    std::chrono::time_point<std::chrono::steady_clock> mLapStart;

    // A vector that holds the time (in seconds) of each lap.
    std::vector<double> mLaps{0.0};
};
} // namespace cse498

#endif // TIMER_H
