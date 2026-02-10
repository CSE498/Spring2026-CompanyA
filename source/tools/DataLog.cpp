#include "DataLog.hpp"
#include <vector>
#include <cstddef>
#include <expected>
#include <string>
#include <utility>
#include <algorithm>

void DataLog::Add(double value, double timestamp){
    //use the timer class to get the timestamp
    std::pair<double, double> combined_data = {value, timestamp};
    data_values.push_back(combined_data);
}


void DataLog::Clear(){
    data_values.clear();
}

std::size_t DataLog::Count(){
    return data_values.size();
}

std::expected<double, std::string> DataLog::Min(){

    if(data_values.empty()){
        return std::unexpected("Data is empty.");
    }

    double min = data_values[0].first;

    for(const auto& val : data_values){
        if(val.first < min){
            min = val.first;
        }
    }
    return min;
}

std::expected<double, std::string> DataLog::Max(){
    if(data_values.empty()){
        return std::unexpected("Data is empty.");
    }

    double max = data_values[0].first;

    for(const auto& val : data_values){
        if(val.first > max){
            max = val.first;
        }
    }

    return max;
}

std::expected<double, std::string> DataLog::Mean(){

    if(data_values.empty()){
        return std::unexpected("Data is empty.");
    }

    double sum = 0.0;
    for(const auto& val : data_values){
        sum += val.first;
    }

    double num_size = static_cast<double>(data_values.size());

    return sum / num_size;
}

std::expected<double, std::string> DataLog::Median(){
    if(data_values.empty()){
        return std::unexpected("Data is empty.");
    }

    std::vector<double> stored_data;

    for(const auto& val : data_values){
        stored_data.push_back(val.first);
    }

    std::sort(stored_data.begin(),stored_data.end());

    const auto size = stored_data.size();
    const auto midpoint = size/2;

    //check if odd
    if(size % 2 == 1){
        return stored_data[midpoint];
    }
    
    double median = (stored_data[midpoint] + stored_data[midpoint - 1]) / 2.0;
    return median;
    
}
