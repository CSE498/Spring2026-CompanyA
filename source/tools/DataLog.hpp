/**
 * @file DataLog.hpp
 * @brief Time-stamped sequence of numeric samples with basic statistics and threshold timing.
 * @author Aneesh Joshi
 * @note Status: PROPOSAL
 *
 * Each sample stores a value and elapsed seconds since construction (via an internal Timer).
 * The caller adds values; timestamps are recorded automatically.
 */
#pragma once
#include <cstddef>
#include <optional>
#include <vector>
#include "Timer.hpp"

namespace cse498 {

/**
 * @brief Append-only log of (value, timestamp) pairs relative to construction time.
 */
class DataLog {
public:
    /** @brief One recorded sample. */
    struct DataSample {
        double value; ///< Sampled value.
        double timestamp; ///< Seconds since this DataLog was constructed.
    };

    /// @brief Construct an empty log with a fresh timer.
    DataLog();

    /// @brief Append @p value with timestamp from the internal timer.
    void Add(double value);

    /// @brief All samples in insertion order.
    const std::vector<DataSample>& DataSamples() const;

    /// @brief Remove all samples (timer state is unchanged).
    void Clear();

    /// @brief Number of stored samples.
    std::size_t Count() const;

    /// @brief Smallest value, or nullopt if empty.
    std::optional<double> Min() const;

    /// @brief Largest value, or nullopt if empty.
    std::optional<double> Max() const;

    /// @brief Arithmetic mean of values, or nullopt if empty.
    std::optional<double> Mean() const;

    /// @brief Median of values, or nullopt if empty.
    std::optional<double> Median() const;

    /**
     * @brief Total duration (in seconds) for which consecutive samples stayed strictly below @p threshold.
     * @param threshold Comparison threshold for the "under" intervals.
     */
    double TimeUnderThreshold(double threshold) const;

    /**
     * @brief Total duration (in seconds) for which consecutive samples stayed strictly above @p threshold.
     * @param threshold Comparison threshold for the "over" intervals.
     */
    double TimeOverThreshold(double threshold) const;

    /// @brief Advance the internal timer by @p seconds (for tests).
    void advanceTimeForTesting(double seconds);

private:
    std::vector<DataSample> mDataValues;
    Timer mTimer{"DataLog"};
};
} // namespace cse498
