#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <numeric>

namespace cse498 {

/**
 * @file FeatureVector.hpp
 * @brief Fixed-size feature vector indexed by an enum.
 *
 * `FeatureVector` is parameterized by a `FeatureEnum` that defines the set of
 * features stored in the vector. The enum must:
 * - Be an `enum class`
 * - Start at 0
 * - Be contiguous (no gaps)
 * - End with a `COUNT` value representing the number of features
 *
 * Each enum value corresponds to a fixed index in the vector, ensuring
 * that all `FeatureVector`s using the same enum share the same layout.
 *
 * Example:
 *
 * @code
 * enum class AgentFeature {
 *     Health = 0,
 *     Damage,
 *     DistanceToGoal,
 *     NearbyEnemies,
 *     COUNT
 * };
 *
 * FeatureVector<AgentFeature> fv;
 * fv.set(AgentFeature::Health, 100.0);
 * fv.set(AgentFeature::Damage, 75.0);
 *
 * double health = fv.get(AgentFeature::Health);
 * @endcode
 */

/**
 * @brief A fixed-size vector of `double` values indexed by `FeatureEnum`.
 * @tparam FeatureEnum Enum type describing the layout (must end with `COUNT`).
 */
template<typename FeatureEnum>
class FeatureVector {

    static constexpr std::size_t feature_count = static_cast<std::size_t>(FeatureEnum::COUNT);

    std::array<double, feature_count> values_{};

    static constexpr std::size_t to_index(FeatureEnum feature) noexcept { return static_cast<std::size_t>(feature); }

public:
    FeatureVector() = default;

    /**
     * @brief Construct from a list of feature values.
     * @param init Values in enum index order; must contain exactly `size()` elements.
     */
    explicit FeatureVector(std::initializer_list<double> init) {
        assert(init.size() == feature_count && "FeatureVector: wrong number of initial values");
        std::size_t i = 0;
        for (double value: init) {
            values_[i++] = value;
        }
    }

    /**
     * @brief Number of features in this vector.
     * @return The constant feature count (`FeatureEnum::COUNT`).
     */
    [[nodiscard]] constexpr std::size_t size() const noexcept { return feature_count; }

    /**
     * @brief Get the value for a feature.
     * @param feature Feature identifier.
     * @return Stored value.
     */
    [[nodiscard]] double get(FeatureEnum feature) const noexcept {
        assert(to_index(feature) < feature_count && "FeatureVector::get feature out of range");
        return values_[to_index(feature)];
    }

    /**
     * @brief Set the value for a feature.
     * @param feature Feature identifier.
     * @param value Value to store.
     */
    void set(FeatureEnum feature, double value) noexcept {
        assert(to_index(feature) < feature_count && "FeatureVector::set feature out of range");
        values_[to_index(feature)] = value;
    }

    /**
     * @brief Get a value by raw index.
     * @param index Index in \([0, size())\).
     * @return Stored value.
     */
    [[nodiscard]] double at(std::size_t index) const noexcept {
        assert(index < feature_count && "FeatureVector::at index out of range");
        return values_[index];
    }

    /**
     * @brief Compare element-wise equality.
     */
    [[nodiscard]] bool operator==(const FeatureVector& other) const noexcept { return values_ == other.values_; }

    /**
     * @brief Negation of `operator==`.
     */
    [[nodiscard]] bool operator!=(const FeatureVector& other) const noexcept { return !(*this == other); }

    /**
     * @brief Compute dot product with another vector.
     * @param other Vector with the same layout.
     * @return \(\sum_i this[i] \cdot other[i]\).
     */
    [[nodiscard]] double dot(const FeatureVector& other) const noexcept {
        return std::inner_product(values_.begin(), values_.end(), other.values_.begin(), 0.0);
    }

    /**
     * @brief Sum of all elements.
     * @return \(\sum_i this[i]\).
     */
    [[nodiscard]] double sum() const noexcept { return std::accumulate(values_.begin(), values_.end(), 0.0); }

    /**
     * @brief Element-wise addition.
     */
    [[nodiscard]] FeatureVector operator+(const FeatureVector& other) const noexcept {
        FeatureVector result;
        for (std::size_t i = 0; i < feature_count; ++i)
            result.values_[i] = values_[i] + other.values_[i];
        return result;
    }

    /**
     * @brief Element-wise subtraction.
     */
    [[nodiscard]] FeatureVector operator-(const FeatureVector& other) const noexcept {
        FeatureVector result;
        for (std::size_t i = 0; i < feature_count; ++i)
            result.values_[i] = values_[i] - other.values_[i];
        return result;
    }

    /**
     * @brief Scalar multiplication.
     * @param scalar Value to multiply each element by.
     */
    [[nodiscard]] FeatureVector operator*(double scalar) const noexcept {
        FeatureVector result;
        for (std::size_t i = 0; i < feature_count; ++i)
            result.values_[i] = values_[i] * scalar;
        return result;
    }

    /**
     * @brief Scalar multiplication (commuted).
     */
    [[nodiscard]] friend FeatureVector operator*(double scalar, const FeatureVector& v) noexcept { return v * scalar; }

    /**
     * @brief In-place element-wise addition.
     */
    FeatureVector& operator+=(const FeatureVector& other) noexcept {
        for (std::size_t i = 0; i < feature_count; ++i)
            values_[i] += other.values_[i];
        return *this;
    }

    /**
     * @brief In-place element-wise subtraction.
     */
    FeatureVector& operator-=(const FeatureVector& other) noexcept {
        for (std::size_t i = 0; i < feature_count; ++i)
            values_[i] -= other.values_[i];
        return *this;
    }

    /**
     * @brief In-place scalar multiplication.
     */
    FeatureVector& operator*=(double scalar) noexcept {
        for (double& v: values_)
            v *= scalar;
        return *this;
    }

    /**
     * @brief Normalize to unit length (L2 norm), in-place.
     *
     * If the magnitude is non-positive (e.g., all zeros), this is a no-op.
     */
    void normalize() noexcept {
        const double mag_sq = dot(*this);
        if (mag_sq <= 0.0)
            return;
        const double mag = std::sqrt(mag_sq);
        for (double& v: values_)
            v /= mag;
    }

    /**
     * @brief Element-wise (Hadamard) product.
     * @param other Vector with the same layout.
     * @return Vector where each element is `this[i] * other[i]`.
     */
    [[nodiscard]] FeatureVector hadamard(const FeatureVector& other) const noexcept {
        FeatureVector result;
        for (std::size_t i = 0; i < feature_count; ++i)
            result.values_[i] = values_[i] * other.values_[i];
        return result;
    }

    /**
     * @brief Access the underlying storage.
     * @return Reference to the backing `std::array` in enum index order.
     */
    [[nodiscard]] const std::array<double, feature_count>& data() const noexcept { return values_; }
};

} // namespace cse498
