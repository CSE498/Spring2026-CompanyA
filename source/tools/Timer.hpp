/**
 * @file Timer.hpp
 * @author Nathan Charters
 *
 * A Timer class that can be used to measure precise time intervals.
 */

#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <unordered_map>
#include <string>

namespace cse498
{

    struct TimerInfo
    {
        std::chrono::time_point<std::chrono::steady_clock> startTime;
        bool isRunning;
        std::chrono::duration<double, std::nano> accumulatedDuration;
    };

    class Timer
    {
    public:
        Timer();
        ~Timer() = default;
        void start(std::string timerName);
        void stop(std::string timerName);
        void restart(std::string timerName);
        double getTime(std::string timerName);

        /**
         * Returns whether the timer with the provided name is currently running. If the timer doesn't exist, returns false.
         */
        bool isRunning(std::string timerName) { return mTimers.find(timerName) != mTimers.end() ? mTimers.at(timerName).isRunning : false; }

    private:
        std::unordered_map<std::string, TimerInfo> mTimers = {};
    };
}

#endif // TIMER_H