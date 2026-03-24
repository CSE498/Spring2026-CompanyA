#pragma once

#include <chrono>

#include "Building.hpp"
#include "InteractiveWorldInventory.hpp"

using namespace cse498;

/**
 * Produces a single kind of resource over time. Rate of production is modified by a Building object
 */
class ResourceProducer
{
public:
	/**
	 * Construct the ResourceProducer
	 * @param buildingPtr pointer to the building modifying the output rate
	 * @param inv         pointer to the world inventory
	 * @param itemType    type of item being produced by this producer
	 * @param startRate   base rate of production with no upgrades
	 */
	ResourceProducer(std::shared_ptr<Building> buildingPtr,
			 std::shared_ptr<InteractiveWorldInventory> inv,
			 ItemType itemType,
			 float startRate)
	{
		m_lastTime = std::chrono::steady_clock::now();
		m_outputType = itemType;
		m_baseRate = startRate;
		SetWorldInventory(inv);
		SetBuilding(buildingPtr);
	}
	/**
	 * Get the current rate of production
	 * @return current rate of production
	 */
	float GetRate() { return m_rate; }
	/**
	 * Calculate the current rate of production
	 */
	void CalculateRate()
	{
		if (!m_building)
		{
			m_rate = m_baseRate;
			return;
		}

		float multiplier;
		if (m_building->GetCurrentLevel() == 0)
			multiplier = 1.0;
		else
			multiplier = m_building->GetCurrentLevel() * m_building->GetRateModifier();
		m_rate = m_baseRate * multiplier;
	}

	/**
	 * Update the resource production
	 */
	void Update()
	{
		// Clock current time for delta time
	        auto now = std::chrono::steady_clock::now();
	        std::chrono::duration<float> dt = now - m_lastTime;
	        m_lastTime = now;
		// Produce items
	        m_accumulator += m_rate * dt.count();
		// If there is a whole item in the accumulator then add it to the world inventory
	        int whole = (int)m_accumulator;
	        if(whole > 0)
	        {
			world->GetInventory().AddItem(m_outputType, whole);
		        m_accumulator -= whole;
	        }
	}

private:
    std::shared_ptr<InteractiveWorld> m_world;            // World
    std::shared_ptr<Building> m_building;                 // Building modifying the output rate
    float m_baseRate{};                                   // Base production rate
    float m_rate{};                                       // Current Production Rate
    float m_accumulator{0.0};                             // Accumulated fraction of a resource over time
    ItemType m_outputType;                                // Type of item being produced
    std::chrono::steady_clock::time_point m_lastTime;     // Last time checked for delta time

    /**
     * Set the building that modifies this production
     * @param newBuilding pointer to building that modifies this producer
     */
    void SetBuilding(std::shared_ptr<Building> newBuilding)
    {
	    m_building = newBuilding;
	    // Adjust rate
	    CalculateRate();
    }

    /**
     * Set the world inventory
     * @param inv pointer to world inventory
     */
    void SetWorld(std::shared_ptr<InteractiveWorld> world)
    {
	    m_world = world;
    }
};