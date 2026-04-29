/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents the production of an ItemType over time.
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
 * Produces a single kind of resource over time. Rate of production is modified
 * by a Building object
 */
class ResourceProducer {
public:
    /**
     * Construct the ResourceProducer
     * @param building the building modifying the output rate
     * @param spawn the spawn for this resource
     * @param itemType    type of item being produced by this producer
     * @param startRate   base rate of production with no upgrades
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
     * Get the current rate of production
     * @return current rate of production
     */
    float GetRate() const { return m_rate; }
    /**
     * Calculate the current rate of production
     */
    void CalculateRate() {
        m_rate = static_cast<float>(GetBurstQuantity()) / GetBurstIntervalSeconds();
    }

    /**
     * Update the resource production
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

    std::reference_wrapper<ResourceSpawn> m_resourceSpawn;
    std::reference_wrapper<Building> m_building; // Building modifying the output rate
    int m_baseBurstQuantity{}; // Base resources created per interval
    float m_rate{}; // Display rate as resources per second
    std::chrono::steady_clock::duration m_burstInterval;
    std::chrono::steady_clock::time_point m_lastTime; // Last time checked for delta time
};
} // namespace cse498
