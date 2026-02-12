
#pragma once
#include <vector>
#include <cstddef>
#include <expected>
#include <string>
#include <utility>
#include <algorithm>
#include <chrono>


class DataLog{
private:
    //(data_value, timestamp)
    std::vector<std::pair<double, double>> data_values;

    std::chrono::steady_clock::time_point start_timestamp;

public:
    DataLog();
    //void Add(double value, double timestamp);
    void Add(double value);
    const std::vector<std::pair<double,double>>& DataSamples() const;
    void Clear();
    std::size_t Count();
    std::expected<double, std::string> Min();
    std::expected<double, std::string> Max();
    std::expected<double, std::string> Mean();
    std::expected<double, std::string> Median();

};