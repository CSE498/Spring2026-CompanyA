#pragma once
#include <cassert>
#include <cmath>
#include <cstddef>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace cse498
{

    /// A numeric vector supporting common feature-space operations
    /// such as dot product and normalization.
    /// @tparam T  The scalar type stored in the vector (e.g. double, float).
    template <typename T>
    class FeatureVector
    {

        private:
            std::vector<T> values_;

        public:
            /// Construct from an existing std::vector, taking ownership via move.
            /// @param values  The vector of elements to store.
            explicit FeatureVector(std::vector<T> values)
                : values_(std::move(values)) {}

            /// Construct from a raw pointer and length.
            /// @param data  Pointer to the first element.
            /// @param size  Number of elements to copy.
            FeatureVector(const T* data, std::size_t size)
                : values_(data, data + size) {}

            /// @return The number of elements in this vector.
            [[nodiscard]] std::size_t size() const noexcept { return values_.size(); }

            /// Bounds-checked element access.
            /// @param index  Position of the element to retrieve.
            /// @return The value at the given index.
            /// @throws std::out_of_range if index >= size().
            [[nodiscard]] T at(std::size_t index) const
            {
                if (index >= values_.size())
                {
                    throw std::out_of_range("FeatureVector::at index out of range");
                }
                return values_[index];
            }

            /// Compute the dot (inner) product of this vector with another.
            /// Both vectors must have the same size (checked via assert).
            /// @param other  The vector to dot with.
            /// @return The scalar dot product.
            [[nodiscard]] T dot(const FeatureVector& other) const
            {
                assert(values_.size() == other.values_.size());
                return std::inner_product(values_.begin(), values_.end(),
                                          other.values_.begin(), T{0});
            }

            /// Normalize this vector to unit length (L2 norm).
            /// If the norm is zero the vector is left unchanged.
            void normalize()
            {
                const T sum_sq = std::inner_product(values_.begin(), values_.end(),
                                              values_.begin(), T{0});
                const T norm = std::sqrt(sum_sq);

                if (norm == T{0})
                {
                    return;
                }

                for (auto& v : values_)
                {
                    v /= norm;
                }
            }

            /// Normalize this vector to unit length then multiply every
            /// element by scale_val. Returns *this to allow chaining.
            /// @param scale_val  The scalar to multiply by after normalizing.
            /// @return A reference to this vector.
            FeatureVector& scale(double scale_val)
            {
                normalize();

                for (auto& v : values_)
                {
                    v *= static_cast<T>(scale_val);
                }

                return *this;
            }

            /// Rotate the vector around the axis defined by the two indices i and j.
            /// @param i  The first index.
            /// @param j  The second index.
            /// @param theta  The angle to rotate by.
            /// @return A reference to this vector.
            FeatureVector& rotate(std::size_t i, std::size_t j, double theta)
            {
                if (i >= values_.size() || j >= values_.size())
                {
                    throw std::out_of_range("FeatureVector::rotate indices out of range");
                }

                T cos_theta = std::cos(theta);
                T sin_theta = std::sin(theta);

                T xi = values_[i];
                T yi = values_[j];

                values_[i] = cos_theta * xi - sin_theta * yi;
                values_[j] = sin_theta * xi + cos_theta * yi;
                return *this;
            }
            // FeatureVector operator+(const FeatureVector& other) const
            // {
            //     if (values_.size() != other.values_.size())
            //     {
            //         throw std::invalid_argument("FeatureVector::operator+ vectors must be the same size");
            //     }
            //     return FeatureVector(values_ + other.values_);
            // }

            // FeatureVector operator-(const FeatureVector& other) const
            // {
            //     if (values_.size() != other.values_.size())
            //     {
            //         throw std::invalid_argument("FeatureVector::operator- vectors must be the same size");
            //     }
            //     return FeatureVector(values_ - other.values_);
            // }

            /// Compute the Hadamard (element-wise) product of this vector with another.
            /// Both vectors must have the same size.
            /// @param other  The vector to multiply element-wise with.
            /// @return A new FeatureVector containing the element-wise products.
            /// @throws std::invalid_argument if the vectors differ in size.
            [[nodiscard]] FeatureVector hadamard(const FeatureVector& other) const
            {
                if (values_.size() != other.values_.size())
                {
                    throw std::invalid_argument("FeatureVector::hadamard vectors must be the same size");
                }

                std::vector<T> result(values_.size());
                for (std::size_t i = 0; i < values_.size(); ++i)
                {
                    result[i] = values_[i] * other.values_[i];
                }

                return FeatureVector(std::move(result));
            }

            /// Apply a unary function to each element in-place.
            /// @tparam Func  Callable taking T and returning T.
            /// @param func   The function to apply.
            /// @return Reference to this vector for chaining.
            template <typename Func>
            FeatureVector& transform(Func && func)
            {
                for (auto& v : values_)
                {
                    v = func(v);
                }
                return *this;
            }

            /// Create a new vector by applying a unary function to each element.
            /// @tparam Func  Callable taking T and returning T.
            /// @param func   The function to apply.
            /// @return A new FeatureVector with transformed values.
            template <typename Func>
            [[nodiscard]] FeatureVector map(Func && func) const
            {
                std::vector<T> result(values_.size());
                for (std::size_t i = 0; i < values_.size(); ++i)
                {
                    result[i] = func(values_[i]);
                }
                return FeatureVector(std::move(result));
            }

            /// Reduce the vector to a single value using a binary function.
            /// @tparam Func  Callable taking (T, T) and returning T.
            /// @param init   Initial accumulator value.
            /// @param func   The binary function to apply.
            /// @return The reduced value.
            template <typename Func>
            [[nodiscard]] T reduce(T init, Func && func) const
            {
                for (const auto& v : values_)
                {
                    init = func(init, v);
                }
                return init;
            }

            /// Apply a binary function element-wise with another vector.
            /// @tparam Func  Callable taking (T, T) and returning T.
            /// @param other  The other vector.
            /// @param func   The binary function to apply.
            /// @return A new FeatureVector with the results.
            template <typename Func>
            [[nodiscard]] FeatureVector apply(const FeatureVector& other, Func && func) const
            {
                assert(values_.size() == other.values_.size());
                std::vector<T> result(values_.size());
                for (std::size_t i = 0; i < values_.size(); ++i)
                {
                    result[i] = func(values_[i], other.values_[i]);
                }
                return FeatureVector(std::move(result));
            }
    };

} // namespace cse498

