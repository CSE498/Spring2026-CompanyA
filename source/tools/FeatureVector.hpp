#pragma once
#include <cstddef>
#include <vector>

namespace cse498
{

class FeatureVector
{

    private:
        std::vector <double> values_;

    public:
        explicit FeatureVector(std::vector <double> values);

        FeatureVector(const double* data, std::size_t size); //Main constructor

        std::size_t const size() const noexcept {return values_.size();}

        double at(std::size_t index) const; //Access

        double dot(const FeatureVector& other) const;

        void normalize();
};

} // namespace cse498

