#include "FeatureVector.h"

#include <cassert>
#include <stdexcept>
#include <cmath>
#include <numeric> //inner_product
#include <iostream>


FeatureVector::FeatureVector(std::vector <double> values) 
: values_(std::move(values)) 
{

}


double FeatureVector::at(std::size_t index) const
{
    if (index >= values_.size())
    {
        throw std::out_of_range("FeatureVector::at index out of range");
    }

    return values_[index];
}


double FeatureVector::dot(const FeatureVector& other) const
{

    assert(values_.size() == other.values_.size());
    return std::inner_product(values_.begin(), values_.end(), other.values_.begin(), 0.0);
}


void FeatureVector::normalize()
{
    double sum_sq = std::inner_product(values_.begin(), values_.end(), values_.begin(), 0.0);

    double norm = std::sqrt(sum_sq);

    if (norm == 0.0)
    {
        return;
    }

    for (double& v: values_)
    {
        v /= norm;
    }
}