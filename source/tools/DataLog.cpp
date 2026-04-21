/**
 * @file DataLog.cpp
 * @brief Implementations for DataLog.
 * @author Aneesh Joshi
 * @note Status: PROPOSAL
 *
 * Min, Max, Mean, and threshold-related logic were developed with AI assistance.
 */

#include "DataLog.hpp"

#include <algorithm>
#include <numeric>

namespace cse498 {

DataLog::DataLog() = default;

void DataLog::Add(double value) {
    DataSample sample;
    sample.value = value;
    sample.timestamp = mTimer.elapsed();
    mDataValues.push_back(sample);
}

const std::vector<DataLog::DataSample>& DataLog::DataSamples() const { return mDataValues; }

void DataLog::Clear() { mDataValues.clear(); }

std::size_t DataLog::Count() const { return mDataValues.size(); }

std::optional<double> DataLog::Min() const {

    if (mDataValues.empty()) {
        return std::nullopt;
    }

    auto min =
            std::min_element(mDataValues.begin(), mDataValues.end(),
                             [](const DataSample& left, const DataSample& right) { return left.value < right.value; });

    return min->value;
}

std::optional<double> DataLog::Max() const {
    if (mDataValues.empty()) {
        return std::nullopt;
    }

    auto max =
            std::max_element(mDataValues.begin(), mDataValues.end(),
                             [](const DataSample& left, const DataSample& right) { return left.value < right.value; });

    return max->value;
}

std::optional<double> DataLog::Mean() const {

    if (mDataValues.empty()) {
        return std::nullopt;
    }

    auto sum = std::accumulate(mDataValues.begin(), mDataValues.end(), 0.0,
                               [](double total, const DataSample& val) { return total + val.value; });

    double num_size = static_cast<double>(mDataValues.size());

    return sum / num_size;
}

std::optional<double> DataLog::Median() const {
    if (mDataValues.empty()) {
        return std::nullopt;
    }

    // Get values from the data log to sort them to find the median
    std::vector<double> stored_data;

    for (const auto& val: mDataValues) {
        stored_data.push_back(val.value);
    }

    std::sort(stored_data.begin(), stored_data.end());

    const auto size = stored_data.size();
    const auto midpoint = size / 2;

    // check if the size of the data log is odd or even to calculate the median
    if (size % 2 == 1) {
        return stored_data[midpoint];
    }

    // when even find the average of the two middle values
    double median = (stored_data[midpoint] + stored_data[midpoint - 1]) / 2.0;
    return median;
}

double DataLog::TimeUnderThreshold(double threshold) const {
    // 2 samples needed for an interval
    if (mDataValues.size() < 2) {
        return 0.0;
    }

    double total_time = 0.0;
    for (std::size_t i = 0; i + 1 < mDataValues.size(); ++i) {
        const auto& current_sample = mDataValues[i];
        const auto& next_sample = mDataValues[i + 1];

        // duration that the current value held
        const double time_diff = next_sample.timestamp - current_sample.timestamp;

        // if the value during [t_cur, t_next) is under the threshold, add the duration to total_time
        if (current_sample.value < threshold) {
            total_time += time_diff;
        }
    }

    return total_time;
}

double DataLog::TimeOverThreshold(double threshold) const {
    // 2 samples needed for an interval
    if (mDataValues.size() < 2) {
        return 0.0;
    }

    double total_time = 0.0;
    for (std::size_t i = 0; i + 1 < mDataValues.size(); ++i) {
        const auto& current_sample = mDataValues[i];
        const auto& next_sample = mDataValues[i + 1];

        // duration that the current value held
        const double time_diff = next_sample.timestamp - current_sample.timestamp;

        // if the value during [t_cur, t_next) is over the threshold, add the duration to total_time
        if (current_sample.value > threshold) {
            total_time += time_diff;
        }
    }

    return total_time;
}

void DataLog::advanceTimeForTesting(double seconds) { mTimer.advanceTime(seconds); }

} // namespace cse498
