/**
 *
 * @author Aneesh Joshi
 * @note Status: PROPOSAL
 *
 * The goal of this class is to provide a time based sequence of numeric values and provides statistics on them.
 * Samples are stored in the format of (Value, timestamp/seconds since start)
 * Caller will add a numeric value and the class will associate a timestamp from when the instance was constructed.
 **/

#pragma once
#include <vector>
#include <cstddef>
#include <optional>
#include "Timer.hpp"

namespace cse498
{
    class DataLog
    {
    public:
        /*
        A struct to represent a recorded sample in the log
        - value: the numeric value of the sample
        - timestamp: seconds since the datalog instance was constructed
        */
        struct DataSample
        {
            double value;
            double timestamp;
        };

        /*
        Constructor for DataLog
        */
        DataLog();

        /*
        Adds a new data value and the function associates a timestamp with the data
        */
        void Add(double value);

        /*
        Function returns a const reference to the collection of data samples
        */
        const std::vector<DataSample> &DataSamples() const;

        /*
        Function clears all samples from the data log
        Timestamp is not reset
        */
        void Clear();

        /*
        Function returns the number of samples stored in the data log
        */
        std::size_t Count() const;

        /*
        Function returns the smallest value in the data log
        */
        std::optional<double> Min() const;

        /*
        Function returns the largest value in the data log
        */
        std::optional<double> Max() const;

        /*
        Function returns the average of the values in the data log/Arithmetic mean
        */
        std::optional<double> Mean() const;

        /*
        Function returns the median of the values in the data log
        */
        std::optional<double> Median() const;

        /*
        Function returns the total time that the values in datalog were under a specific threshold
        */
        double TimeUnderThreshold(double threshold) const;

        /*
        Function returns the total time that the values in datalog were over a specific threshold
        */
        double TimeOverThreshold(double threshold) const;

        /*
        Helper function whose purpose is to advance the Timer for timestamp testing purposes without manually waiting
        */
        void advanceTimeForTesting(double seconds);

    private:
        std::vector<DataSample> mDataValues;

        // Timer to measure elapsed time since datalog was constructed
        Timer mTimer{"DataLog"};
    };
}