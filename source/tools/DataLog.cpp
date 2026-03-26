/**
 *
 * @author Aneesh Joshi
 * @note Status: PROPOSAL
 *
 * The goal of this class is to provide a time based sequence of numeric values
 * and provides statistics on them. Samples are stored in the format of (Value,
 * timestamp/seconds since start) Caller will add a numeric value and the class
 * will associate a timestamp from when the instance was constructed. Used AI to
 * help refactor the Min, Max, and Mean functions to use algorithms
 **/

#include "DataLog.hpp"

#include <algorithm>
#include <numeric>

namespace cse498 {

/*
Constructs a Datalog class and sets the start_timestamp to now
*/
DataLog::DataLog() : start_timestamp(std::chrono::steady_clock::now()) {}

/*
Adds a new data value and the fuction associates a timestamp with the data
Used AI to help create duration in seconds using std::chrono
*/
void DataLog::Add(double value) {
  auto current_timestamp = std::chrono::steady_clock::now();
  // Calculate the duration in seconds since the start timestamp
  auto duration = current_timestamp - start_timestamp;
  std::pair<double, double> combined_data = {
      value, std::chrono::duration<double>(duration).count()};
  data_values.push_back(combined_data);
}

/*
Function returns a reference to the collection of data samples in the format
(data_value, timestamp)
*/
const std::vector<std::pair<double, double>> &DataLog::DataSamples() const {
  return data_values;
}

/*
Function clears all samples from the data log
Timestamp is not reset
*/
void DataLog::Clear() { data_values.clear(); }

/*
Function returns the number of samples stored in the data log
*/
std::size_t DataLog::Count() const { return data_values.size(); }

/*
Function returns the smallest value in the data log
*/
std::optional<double> DataLog::Min() const {

  if (data_values.empty()) {
    return std::nullopt;
  }

  auto min = std::min_element(data_values.begin(), data_values.end(),
                              [](const auto &left, const auto &right) {
                                return left.first < right.first;
                              });

  return min->first;
}

/*
Function returns the largest value in the data log
*/
std::optional<double> DataLog::Max() const {
  if (data_values.empty()) {
    return std::nullopt;
  }

  auto max = std::max_element(data_values.begin(), data_values.end(),
                              [](const auto &left, const auto &right) {
                                return left.first < right.first;
                              });

  return max->first;
}

/*
Function returns the average of the values in the data log
*/
std::optional<double> DataLog::Mean() const {

  if (data_values.empty()) {
    return std::nullopt;
  }

  auto sum = std::accumulate(
      data_values.begin(), data_values.end(), 0.0,
      [](double total, const auto &val) { return total + val.first; });

  double num_size = static_cast<double>(data_values.size());

  return sum / num_size;
}

/*
Function returns the median of the values in the data log
*/
std::optional<double> DataLog::Median() const {
  if (data_values.empty()) {
    return std::nullopt;
  }

  // Get values from the data log to sort them to find the median
  std::vector<double> stored_data;

  for (const auto &val : data_values) {
    stored_data.push_back(val.first);
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

} // namespace cse498