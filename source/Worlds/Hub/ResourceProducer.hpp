/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Burst-based production of an ItemType over time.
 **/

#pragma once

#include <algorithm>
#include <chrono>
#include <cmath>
#include <functional>
#include <memory>

#include "Building.hpp"
#include "ResourceSpawn.hpp"

namespace cse498 {
/**
 * @class ResourceProducer
 * @brief Adds one resource type to a ResourceSpawn in timed bursts.
 *
 * A producer stores fractional design input as an average rate, but runtime
 * production is intentionally burst-based: no resources are added until the
 * configured interval elapses, then a whole burst is deposited into the spawn.
 * The paired Building's current level scales the burst quantity through the
 * building rate modifier.
 */
class ResourceProducer {
public:
    /**
     * @brief Construct a ResourceProducer.
     *
     * @param building Building whose level modifies burst output.
     * @param spawn Spawn that receives produced resources.
     * @param itemType Type of item being produced; retained for call-site clarity.
     * @param startRate Average base production rate in resources per second.
     * @param burstInterval Time between production bursts.
     */
    ResourceProducer(Building& building, ResourceSpawn& spawn, ItemType itemType, float startRate,
                     std::chrono::steady_clock::duration burstInterval = std::chrono::seconds(5)) :
        m_resourceSpawn(spawn), m_building(building), m_burstInterval(burstInterval) {
        static_cast<void>(itemType);
        m_lastTime = std::chrono::steady_clock::now();
        m_baseBurstQuantity = std::max(5, static_cast<int>(std::round(startRate * GetBurstIntervalSeconds())));
        CalculateRate();
    }
    /**
     * @brief Get the current average production rate.
     * @return Current effective resources per second after upgrade scaling.
     */
    float GetRate() const { return m_rate; }
    /**
     * @brief Recalculate the display/analytics rate from the current burst size.
     */
    void CalculateRate() {
        m_rate = static_cast<float>(GetBurstQuantity()) / GetBurstIntervalSeconds();
    }

    /**
     * @brief Add burst resources to the spawn if one or more intervals elapsed.
     *
     * Multiple missed intervals are caught up in one call. This keeps production
     * consistent if frame timing stalls or the GUI pauses briefly.
     */
    void Update() {
        // Refresh rate each tick so building upgrades take effect immediately.
        CalculateRate();

        // Get current time
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - m_lastTime;
        if (elapsed < m_burstInterval) {
            return;
        }

        const auto bursts = elapsed / m_burstInterval;
        m_lastTime += m_burstInterval * bursts;
        m_resourceSpawn.get().AddResource(GetBurstQuantity() * static_cast<int>(bursts));
    }

private:
    [[nodiscard]] float GetBurstIntervalSeconds() const {
        return std::chrono::duration<float>(m_burstInterval).count();
    }

    [[nodiscard]] int GetBurstQuantity() const {
        float multiplier = 1.0f + (m_building.get().GetCurrentLevel() * m_building.get().GetRateModifier());
        return std::max(1, static_cast<int>(std::round(static_cast<float>(m_baseBurstQuantity) * multiplier)));
    }

    std::reference_wrapper<ResourceSpawn> m_resourceSpawn; ///< Spawn receiving produced resources.
    std::reference_wrapper<Building> m_building; ///< Building modifying burst output.
    int m_baseBurstQuantity{}; ///< Base resources created per interval before upgrades.
    float m_rate{}; ///< Effective average rate as resources per second.
    std::chrono::steady_clock::duration m_burstInterval; ///< Time between production bursts.
    std::chrono::steady_clock::time_point m_lastTime; ///< Last burst accounting time.
};
} // namespace cse498
