/**
 * @file Stopwatch.hpp
 * @author Nathan Charters
 *
 * A class representing a Stopwatch object.
 */

#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>
#include <string>
#include <vector>

namespace cse498
{
    /**
     * A class representing a Stopwatch object.
     *
     * A Stopwatch keeps track of a single timing instance, and can be started, stopped, reset, and restarted.
     */
    class Stopwatch
    {
    public:
        Stopwatch(std::string name, bool startRunning = true);

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
        // The name of the Stopwatch.
        std::string mName;

        // The starting time point of the Stopwatch.
        std::chrono::time_point<std::chrono::steady_clock> mStart;

        // The total elapsed time of the Stopwatch. Initialized to 0.
        std::chrono::duration<double, std::nano> mElapsed{std::chrono::duration<double>::zero()};

        // A boolean representing whether the Stopwatch is running or not.
        bool mRunning;

        // The starting time point of the current lap.
        std::chrono::time_point<std::chrono::steady_clock> mLapStart;

        // A vector that holds the time (in seconds) of each lap.
        std::vector<double> mLaps{0.0};
    };
}

#endif // STOPWATCH_H