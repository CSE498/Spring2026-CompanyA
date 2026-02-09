#pragma once
#include <vector>
#include <cstddef>
#include <expected>
#include <string>
#include <utility>
#include <algorithm>


class DataLog{
private:
    //(data_value, timestamp)
    std::vector<std::pair<double, double>> data_values;

public:
    void Add(double value, double timestamp);
    void Clear();
    std::size_t Count();
    std::expected<double, std::string> Min();
    std::expected<double, std::string> Max();
    std::expected<double, std::string> Mean();
    std::expected<double, std::string> Median();

};