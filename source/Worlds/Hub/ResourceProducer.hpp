/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents the production of an ItemType over time.
 **/

#pragma once

#include <chrono>
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
    ResourceProducer(Building& building, ResourceSpawn& spawn, ItemType itemType, float startRate) :
        m_resourceSpawn(spawn), m_building(building) {
        static_cast<void>(itemType);
        m_lastTime = std::chrono::steady_clock::now();
        m_baseRate = startRate;
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
        // multiplier = 1 + current level * rate modifier
        // If rate modifier is 0.25 then it will add .25 per level
        // A level 3 building with a rate modifier of 0.25 will be
        // 1 + 3 * 0.25 = 1.75x production
        float multiplier = 1.0f + (m_building.get().GetCurrentLevel() * m_building.get().GetRateModifier());
        m_rate = m_baseRate * multiplier;
    }

    /**
     * Update the resource production
     */
    void Update() {
        // Refresh rate each tick so building upgrades take effect immediately.
        CalculateRate();

        // Get current time
        auto now = std::chrono::steady_clock::now();
        // Get delta time since last update
        std::chrono::duration<float> dt = now - m_lastTime;
        m_lastTime = now;
        // add the amount of resources produced since last update as a float
        m_accumulator += m_rate * dt.count();
        // If the amount added is greater than or equal to 1 then add the whole
        // integer to the world inventory and subtract the amount added from the
        // accumulator
        int whole = static_cast<int>(m_accumulator);
        if (whole > 0) {
            m_resourceSpawn.get().AddResource(whole);
            m_accumulator -= whole;
        }
    }

private:
    std::reference_wrapper<ResourceSpawn> m_resourceSpawn;
    std::reference_wrapper<Building> m_building; // Building modifying the output rate
    float m_baseRate{}; // Base production rate
    float m_rate{}; // Current Production Rate
    float m_accumulator{0.0f}; // Accumulated fraction of a resource over time
    std::chrono::steady_clock::time_point m_lastTime; // Last time checked for delta time
};
} // namespace cse498
