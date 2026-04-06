/**
 * @file Timer.cpp
 * @author Nathan Charters
 */

#include "Timer.hpp"

// The elapsed time is calculated internally in nanoseconds.
// This variable allows for simple conversion in methods that return a value in
// seconds.
constexpr double NS_TO_S_CONVERSION = 1000000000.0;

/**
 * Timer constructor.
 * @param name The name of the Timer.
 * @param startRunning Whether the Timer should start running. Defaults to true.
 */
cse498::Timer::Timer(std::string name, bool startRunning) {
  mName = name;
  mRunning = startRunning;
  mStart = std::chrono::steady_clock::now();
  mLapStart = std::chrono::steady_clock::now();
}

/**
 * Start the Timer.
 * If the Timer is already running, it won't do anything.
 */
void cse498::Timer::start() {
  if (!mRunning) {
    mRunning = true;
    mStart = std::chrono::steady_clock::now();
    mLapStart = std::chrono::steady_clock::now();
  }
}

/**
 * Stop the Timer and update the elapsed time and current lap time.
 * If the Timer is already stopped, it won't do anything.
 */
void cse498::Timer::stop() {
  if (mRunning) {
    mRunning = false;
    mElapsed += (std::chrono::steady_clock::now() - mStart);
    mLaps.at(mLaps.size() - 1) +=
        (std::chrono::steady_clock::now() - mLapStart).count() /
        NS_TO_S_CONVERSION;
  }
}

/**
 * Reset the Timer, turning it off, setting its elapsed time back to 0, and
 * resetting the laps.
 */
void cse498::Timer::reset() {
  mRunning = false;
  mElapsed = std::chrono::duration<double>::zero();
  mLaps.clear();
  mLaps.push_back(0.0);
}

/**
 * Restart the Timer, resetting its elapsed time back to 0 and resetting the
 * laps, but starting it up immediately.
 */
void cse498::Timer::restart() {
  mElapsed = std::chrono::duration<double>::zero();
  mLaps.clear();
  mLaps.push_back(0.0);
  mRunning = true;
  mStart = std::chrono::steady_clock::now();
  mLapStart = std::chrono::steady_clock::now();
}

/**
 * Lap the Timer, stopping the previous lap and immediately starting a new one.
 */
void cse498::Timer::lap() {
  if (mRunning) {
    mLaps.at(mLaps.size() - 1) +=
        (std::chrono::steady_clock::now() - mLapStart).count() /
        NS_TO_S_CONVERSION;
    mLaps.push_back(0.0);
    mLapStart = std::chrono::steady_clock::now();
  }
}

/**
 * Gets and calculates the total time (in seconds) that the Timer has run for.
 * @return Total run time of Timer (seconds).
 */
double cse498::Timer::elapsed() const {
  if (mRunning) {
    // Subtracts the starting time point from the current time and adds that
    // duration to any already accumulated duration.
    return (mElapsed + (std::chrono::steady_clock::now() - mStart)).count() /
           NS_TO_S_CONVERSION;
  }
  return mElapsed.count() / NS_TO_S_CONVERSION;
}

/**
 * Checks whether the Timer is currently running.
 * @return Whether the Timer is running or not.
 */
bool cse498::Timer::isRunning() const { return mRunning; }

/**
 * Returns the vector containing the lap times for the Timer.
 * If the Timer is running, add the elapsed time into the current lap time to
 * get an accurate lap time, and adjust the starting point as necessary for
 * future calculations.
 */
std::vector<double> cse498::Timer::getLaps() {
  if (mRunning) {
    mLaps.at(mLaps.size() - 1) +=
        (std::chrono::steady_clock::now() - mLapStart).count() /
        NS_TO_S_CONVERSION;
    mLapStart = std::chrono::steady_clock::now();
  }
  return mLaps;
}

/**
 * Mock an advance of time by the given interval (in seconds).
 * This method should only be used for testing purposes, and has no use outside
 * of making sure elapsed() works properly.
 * @param seconds The amount of time to advance by.
 */
void cse498::Timer::advanceTime(double seconds) {
  if (mRunning) {
    mElapsed += std::chrono::duration<double>(seconds);
    mLaps.at(mLaps.size() - 1) +=
        std::chrono::duration<double>(seconds).count();
  }
}

/**
 * Returns a string containing the state of the timer in xx:xx.xxx format
 * (minutes, seconds, milliseconds)
 * @param withLaps Should the string include lap information or not
 */
std::string cse498::Timer::toString(bool withLaps) const {
  double time = elapsed();
  std::ostringstream oss;
  oss << mName << " [" << (mRunning ? "RUNNING" : "STOPPED")
      << "]: " << std::setfill('0') << std::setw(2)
      << static_cast<int>(time / 60) << ":" << std::setw(6) << std::fixed
      << std::setprecision(3) << std::fmod(time, 60.0);

  if (withLaps) {
    for (auto [i, lapTime] : std::ranges::views::enumerate(mLaps)) {
      oss << "\n  Lap " << (i + 1) << ": " << std::setfill('0') << std::setw(2)
          << static_cast<int>(lapTime / 60) << ":" << std::setw(6) << std::fixed
          << std::setprecision(3) << std::fmod(lapTime, 60.0);
    }
  }
  return oss.str();
}