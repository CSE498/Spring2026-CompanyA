/**
 * @file FeatureVector.hpp
 * @brief Fixed-size vector of doubles indexed by a contiguous enum class ("feature vector").
 */
#pragma once

#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <initializer_list>
#include <numeric>

namespace cse498 {

/**
 * @tparam FeatureEnum `enum class` with enumerators `0, 1, …, COUNT-1` and a final `COUNT`
 *         equal to the number of features. Values must be contiguous with no gaps.
 *
 * Each enumerator maps to a fixed index; all FeatureVectors sharing the same enum share layout.
 *
 * Example:
 * @code
 * enum class AgentFeature {
 *     Health = 0,
 *     Damage,
 *     DistanceToGoal,
 *     NearbyEnemies,
 *     COUNT
 * };
 * FeatureVector<AgentFeature> fv;
 * fv.set(AgentFeature::Health, 100.0);
 * @endcode
 */
template<typename FeatureEnum>
class FeatureVector {

    static constexpr std::size_t feature_count = static_cast<std::size_t>(FeatureEnum::COUNT);

    std::array<double, feature_count> values_{};

    static constexpr std::size_t to_index(FeatureEnum feature) noexcept { return static_cast<std::size_t>(feature); }

public:
    /// @brief Zero-initialized vector.
    FeatureVector() = default;

    /**
     * @brief Initialize from exactly `FeatureEnum::COUNT` values in enum order.
     * @param init Initializer list length must equal feature_count (asserted).
     */
    explicit FeatureVector(std::initializer_list<double> init) {
        assert(init.size() == feature_count && "FeatureVector: wrong number of initial values");
        std::size_t i = 0;
        for (double value: init) {
            values_[i++] = value;
        }
    }

    /// @brief Number of features (same as `FeatureEnum::COUNT`).
    [[nodiscard]] constexpr std::size_t size() const noexcept { return feature_count; }

    /// @brief Value at @p feature.
    [[nodiscard]] double get(FeatureEnum feature) const noexcept {
        assert(to_index(feature) < feature_count && "FeatureVector::get feature out of range");
        return values_[to_index(feature)];
    }

    /// @brief Set value at @p feature.
    void set(FeatureEnum feature, double value) noexcept {
        assert(to_index(feature) < feature_count && "FeatureVector::set feature out of range");
        values_[to_index(feature)] = value;
    }

    /// @brief Unchecked access by underlying index (asserted in range).
    [[nodiscard]] double at(std::size_t index) const noexcept {
        assert(index < feature_count && "FeatureVector::at index out of range");
        return values_[index];
    }

    [[nodiscard]] bool operator==(const FeatureVector& other) const noexcept { return values_ == other.values_; }

    [[nodiscard]] bool operator!=(const FeatureVector& other) const noexcept { return !(*this == other); }

    /// @brief Dot product with @p other.
    [[nodiscard]] double dot(const FeatureVector& other) const noexcept {
        return std::inner_product(values_.begin(), values_.end(), other.values_.begin(), 0.0);
    }

    /// @brief Sum of all components.
    [[nodiscard]] double sum() const noexcept { return std::accumulate(values_.begin(), values_.end(), 0.0); }

    [[nodiscard]] FeatureVector operator+(const FeatureVector& other) const noexcept {
        FeatureVector result;
        for (std::size_t i = 0; i < feature_count; ++i)
            result.values_[i] = values_[i] + other.values_[i];
        return result;
    }

    [[nodiscard]] FeatureVector operator-(const FeatureVector& other) const noexcept {
        FeatureVector result;
        for (std::size_t i = 0; i < feature_count; ++i)
            result.values_[i] = values_[i] - other.values_[i];
        return result;
    }

    [[nodiscard]] FeatureVector operator*(double scalar) const noexcept {
        FeatureVector result;
        for (std::size_t i = 0; i < feature_count; ++i)
            result.values_[i] = values_[i] * scalar;
        return result;
    }

    [[nodiscard]] friend FeatureVector operator*(double scalar, const FeatureVector& v) noexcept { return v * scalar; }

    FeatureVector& operator+=(const FeatureVector& other) noexcept {
        for (std::size_t i = 0; i < feature_count; ++i)
            values_[i] += other.values_[i];
        return *this;
    }

    FeatureVector& operator-=(const FeatureVector& other) noexcept {
        for (std::size_t i = 0; i < feature_count; ++i)
            values_[i] -= other.values_[i];
        return *this;
    }

    FeatureVector& operator*=(double scalar) noexcept {
        for (double& v: values_)
            v *= scalar;
        return *this;
    }

    /// @brief In-place L2 normalization; no-op if the vector is zero.
    void normalize() noexcept {
        const double mag_sq = dot(*this);
        if (mag_sq <= 0.0)
            return;
        const double mag = std::sqrt(mag_sq);
        for (double& v: values_)
            v /= mag;
    }

    /// @brief Element-wise product with @p other.
    [[nodiscard]] FeatureVector hadamard(const FeatureVector& other) const noexcept {
        FeatureVector result;
        for (std::size_t i = 0; i < feature_count; ++i)
            result.values_[i] = values_[i] * other.values_[i];
        return result;
    }

    /// @brief Direct read-only access to the backing array.
    [[nodiscard]] const std::array<double, feature_count>& data() const noexcept { return values_; }
};

} // namespace cse498
