/**
 * @file Timer.hpp
 * @author Nathan Charters
 *
 * A class representing a Timer object.
 */

#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <string>
#include <vector>

namespace cse498
{
    /**
     * A class representing a Timer object.
     *
     * A Timer keeps track of a single timing instance, and can be started, stopped, reset, and restarted.
     */
    class Timer
    {
    public:
        Timer(std::string name, bool startRunning = true);

        void start();
        void stop();
        void reset();
        void restart();

        void lap();

        double elapsed() const;
        bool isRunning() const;
        std::vector<double> getLaps();

        void advanceTime(double seconds);

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
}

#endif // TIMER_H