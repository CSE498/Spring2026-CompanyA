/**
 *
 * @author Aneesh Joshi
 * @note Status: PROPOSAL
 *
 * The goal of this class is to provide a time based sequence of numeric values and provides statistics on them.
 * Caller will add a numeric value and the class will associate a timestamp from when the instance was constructed.
 * Used AI to help in developing the Min, Max, Mean, and Threshold functions.
 **/

#include "DataLog.hpp"

#include <algorithm>
#include <numeric>

namespace cse498
{

    /*
    Constructor for DataLog
    */
    DataLog::DataLog() = default;

    /*
    Adds a new data value and the function associates a timestamp with the data
    */
    void DataLog::Add(double value)
    {
        DataSample sample;
        sample.value = value;
        sample.timestamp = mTimer.elapsed();
        mDataValues.push_back(sample);
    }

    /*
    Function returns a reference to the collection of data samples
    */
    const std::vector<DataLog::DataSample> &DataLog::DataSamples() const
    {
        return mDataValues;
    }

    /*
    Function clears all samples from the data log
    Timestamp is not reset
    */
    void DataLog::Clear()
    {
        mDataValues.clear();
    }

    /*
    Function returns the number of samples stored in the data log
    */
    std::size_t DataLog::Count() const
    {
        return mDataValues.size();
    }

    /*
    Function returns the smallest value in the data log
    */
    std::optional<double> DataLog::Min() const
    {

        if (mDataValues.empty())
        {
            return std::nullopt;
        }

        auto min = std::min_element(mDataValues.begin(), mDataValues.end(), [](const DataSample &left, const DataSample &right)
                                    { return left.value < right.value; });

        return min->value;
    }

    /*
    Function returns the largest value in the data log
    */
    std::optional<double> DataLog::Max() const
    {
        if (mDataValues.empty())
        {
            return std::nullopt;
        }

        auto max = std::max_element(mDataValues.begin(), mDataValues.end(), [](const DataSample &left, const DataSample &right)
                                    { return left.value < right.value; });

        return max->value;
    }

    /*
    Function returns the average of the data values in the data log
    */
    std::optional<double> DataLog::Mean() const
    {

        if (mDataValues.empty())
        {
            return std::nullopt;
        }

        auto sum = std::accumulate(mDataValues.begin(), mDataValues.end(), 0.0, [](double total, const DataSample &val)
                                   { return total + val.value; });

        double num_size = static_cast<double>(mDataValues.size());

        return sum / num_size;
    }

    /*
    Function returns the median of the values in the data log
    */
    std::optional<double> DataLog::Median() const
    {
        if (mDataValues.empty())
        {
            return std::nullopt;
        }

        // Get values from the data log to sort them to find the median
        std::vector<double> stored_data;

        for (const auto &val : mDataValues)
        {
            stored_data.push_back(val.value);
        }

        std::sort(stored_data.begin(), stored_data.end());

        const auto size = stored_data.size();
        const auto midpoint = size / 2;

        // check if the size of the data log is odd or even to calculate the median
        if (size % 2 == 1)
        {
            return stored_data[midpoint];
        }

        // when even find the average of the two middle values
        double median = (stored_data[midpoint] + stored_data[midpoint - 1]) / 2.0;
        return median;
    }

    /*
    Function returns the total time that the values in datalog were under a specific threshold
    */
    double DataLog::TimeUnderThreshold(double threshold) const
    {
        // 2 samples needed for an interval
        if (mDataValues.size() < 2)
        {
            return 0.0;
        }

        double total_time = 0.0;
        for (std::size_t i = 0; i + 1 < mDataValues.size(); ++i)
        {
            const auto &current_sample = mDataValues[i];
            const auto &next_sample = mDataValues[i + 1];

            // duration that the current value held
            const double time_diff = next_sample.timestamp - current_sample.timestamp;

            // if the value during [t_cur, t_next) is under the threshold, add the duration to total_time
            if (current_sample.value < threshold)
            {
                total_time += time_diff;
            }
        }

        return total_time;
    }

    /*
    Function returns the total time that the values in datalog were over a specific threshold
    */
    double DataLog::TimeOverThreshold(double threshold) const
    {
        // 2 samples needed for an interval
        if (mDataValues.size() < 2)
        {
            return 0.0;
        }

        double total_time = 0.0;
        for (std::size_t i = 0; i + 1 < mDataValues.size(); ++i)
        {
            const auto &current_sample = mDataValues[i];
            const auto &next_sample = mDataValues[i + 1];

            // duration that the current value held
            const double time_diff = next_sample.timestamp - current_sample.timestamp;

            // if the value during [t_cur, t_next) is over the threshold, add the duration to total_time
            if (current_sample.value > threshold)
            {
                total_time += time_diff;
            }
        }

        return total_time;
    }

    /*
    Helper function whose purpose is to advance the Timer for timestamp testing purposes without manually waiting
    */
    void DataLog::advanceTimeForTesting(double seconds)
    {
        mTimer.advanceTime(seconds);
    }

}