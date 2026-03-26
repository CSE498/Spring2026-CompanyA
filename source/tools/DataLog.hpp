/**
 *
 * @author Aneesh Joshi
 * @note Status: PROPOSAL
 *
 * The goal of this class is to provide a time based sequence of numeric values
 * and provides statistics on them. Samples are stored in the format of (Value,
 * timestamp/seconds since start) Caller will add a numeric value and the class
 * will associate a timestamp from when the instance was constructed.
 **/

#pragma once
#include <chrono>
#include <cstddef>
#include <optional>
#include <utility>
#include <vector>

namespace cse498 {
class DataLog {
private:
  //(data_value, timestamp)
  std::vector<std::pair<double, double>> data_values;

  std::chrono::steady_clock::time_point start_timestamp;

public:
  /*
  Constructs a Datalog class and sets the start_timestamp to now
  */
  DataLog();

  /*
  Adds a new data value and the fuction associates a timestamp with the data
  */
  void Add(double value);

  /*
  Function returns a reference to the collection of data samples in the format
  (data_value, timestamp)
  */
  const std::vector<std::pair<double, double>> &DataSamples() const;

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
  Function returns the average of the values in the data log
  */
  std::optional<double> Mean() const;

  /*
  Function returns the median of the values in the data log
  */
  std::optional<double> Median() const;
};
} // namespace cse498