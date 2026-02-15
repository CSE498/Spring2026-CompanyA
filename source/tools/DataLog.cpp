/**
 * 
 * @author Aneesh Joshi
 * @note Status: PROPOSAL
 * 
 * The goal of this class is to provide a time based sequence of numeric values and provides statistics on them.
 * Samples are stored in the format of (Value, timestamp/seconds since start)
 * Caller will add a numeric value and the class will associate a timestamp from when the instance was constructed.
 **/

#include "DataLog.hpp"
#include <vector>
#include <cstddef>
#include <optional>
#include <string>
#include <utility>
#include <algorithm>
#include <chrono>

/*
Constructs a Datalog class and sets the start_timestamp to now
*/
DataLog::DataLog() : start_timestamp(std::chrono::steady_clock::now()){

}

/*
Adds a new data value and the fuction associates a timestamp with the data
*/
void DataLog::Add(double value){
    auto current_timestamp = std::chrono::steady_clock::now();
    auto duration = current_timestamp - start_timestamp;
    std::pair<double, double> combined_data = {value, std::chrono::duration<double>(duration).count()};
    data_values.push_back(combined_data);
}

/*
Function returns a reference to the collection of data samples in the format (data_value, timestamp)
*/
const std::vector<std::pair<double,double>>& DataLog::DataSamples() const{
    return data_values;
}

/*
Function clears all samples from the data log
Timestamp is not reset
*/
void DataLog::Clear(){
    data_values.clear();
}

/*
Function returns the number of samples stored in the data log
*/
std::size_t DataLog::Count() const{
    return data_values.size();
}

/*
Function returns the smallest value in the data log
*/
std::optional<double> DataLog::Min() const{

    if(data_values.empty()){
        return std::nullopt;
    }

    double min = data_values[0].first;

    for(const auto& val : data_values){
        if(val.first < min){
            min = val.first;
        }
    }
    return min;
}

/*
Function returns the largest value in the data log
*/
std::optional<double> DataLog::Max() const{
    if(data_values.empty()){
        return std::nullopt;
    }

    double max = data_values[0].first;

    for(const auto& val : data_values){
        if(val.first > max){
            max = val.first;
        }
    }

    return max;
}

/*
Function returns the average of the values in the data log
*/
std::optional<double> DataLog::Mean() const{

    if(data_values.empty()){
        return std::nullopt;
    }

    double sum = 0.0;
    for(const auto& val : data_values){
        sum += val.first;
    }

    double num_size = static_cast<double>(data_values.size());

    return sum / num_size;
}

/*
Function returns the median of the values in the data log
*/
std::optional<double> DataLog::Median() const{
    if(data_values.empty()){
        return std::nullopt;
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
