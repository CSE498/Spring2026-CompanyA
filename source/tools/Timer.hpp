/**
 * @file Timer.hpp
 * @author Nathan Charters
 *
 * A Timer class that can be used to measure precise time intervals.
 *
 * Current version assumes that all started timers are constantly running until stopped,
 * and that timers are not restarted after being stopped. Future versions will include
 * more detailed functionality including the ability to stop and start again,
 * and to check the time of a timer without stopping it.
 */

#ifndef TIMER_H
#define TIMER_H

#include <chrono>
#include <unordered_map>
#include <string>

class Timer
{
public:
    Timer();
    void start(std::string timerName);
    std::chrono::duration<double> stop(std::string timerName);

private:
    std::unordered_map<std::string, std::chrono::time_point<std::chrono::steady_clock>> mTimers = {};
};

#endif // TIMER_H