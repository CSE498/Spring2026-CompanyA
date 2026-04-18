/**
 *
 * @author Aneesh Joshi
 * @note Status: PROPOSAL
 *
 * The goal of this class is to provide a time based sequence of numeric values and provides statistics on them.
 * Caller will add a numeric value and the class will associate a timestamp from when the instance was constructed.
 * Used AI to help in developing the Min, Max, Mean, Threshold functions, and function comments
 **/

#pragma once
#include <cstddef>
#include <optional>
#include <vector>
#include "Timer.hpp"

namespace cse498 {
class DataLog {
public:

    /**
     * A struct to represent a recorded sample in the log
     * value: the numeric value of the sample
     * timestamp: seconds since the datalog instance was constructed
     */
    struct DataSample {
        double value;
        double timestamp;
    };

    /**
     * DataLog constructor
     */
    DataLog();

    /**
     * Adds a new data value and the function associates a timestamp with the data
     * @param value The numeric value that is added to the log
     */
    void Add(double value);

    /**
     * Function returns a const reference to the collection of data samples
     * @return A const reference to the vector of DataSample objects stored in the DataLog
     */
    const std::vector<DataSample>& DataSamples() const;

    /**
     * Function clears all samples from the data log
     * Timestamp is not reset
     */
    void Clear();

    /**
     * Function returns the number of samples stored in the data log
     * @return The number of samples in the data log
     */
    std::size_t Count() const;

    /**
     * Function returns the smallest value in the data log
     * @return The smallest value in the data log, or std::nullopt if the log is empty
     */
    std::optional<double> Min() const;

    /**
     * Function returns the largest value in the data log
     * @return The largest value in the data log, or std::nullopt if the log is empty
     */
    std::optional<double> Max() const;

    /**
     * Function returns the average of the data values in the data log
     * @return The average of the values in the data log, or std::nullopt if the log is empty
     */
    std::optional<double> Mean() const;

    /**
     * Function returns the median of the values in the data log
     * @return The median of the values in the data log, or std::nullopt if the log is empty
     */
    std::optional<double> Median() const;

    /**
     * Function returns the total time that the values in datalog were under a specific threshold
     * @param threshold The threshold to compare the data values against
     * @return The total time that the values in datalog were under the specified threshold
     */
    double TimeUnderThreshold(double threshold) const;

    /**
     * Function returns the total time that the values in datalog were over a specific threshold
     * @param threshold The threshold to compare the data values against
     * @return The total time that the values in datalog were over the specified threshold
     */
    double TimeOverThreshold(double threshold) const;

    /**
     * Helper function whose purpose is to advance the Timer for timestamp testing purposes without manually waiting
     * @param seconds The amount of time to advance by
     */
    void advanceTimeForTesting(double seconds);

private:
    /**
     * Vector to store the data samples in the log
     */
    std::vector<DataSample> mDataValues;

    /**
     * Timer to measure elapsed time since datalog was constructed
     */
    Timer mTimer{"DataLog"};
};
} // namespace cse498
