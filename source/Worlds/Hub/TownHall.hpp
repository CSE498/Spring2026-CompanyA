/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Class representing the town hall for interactive world.
 **/

#pragma once

#include "../../Agents/PacingAgent.hpp"
#include "../../core/AgentBase.hpp"
#include "InteractiveWorldInventory.hpp"
#include "ItemType.hpp"
#include "ResourceSpawn.hpp"

#include <map>
#include <vector>

namespace cse498 {
/**
 * @class TownHall
 * @brief Final deposit point for InteractiveWorld resources.
 *
 * TownHall writes delivered resources into the shared InteractiveWorldInventory.
 * Fetch agents use it as the second-stage deposit endpoint for each lane.
 */
class TownHall : public AgentBase {
public:
    /**
     * @brief Construct a TownHall using an existing world inventory.
     * @param id Unique entity id.
     * @param name Entity name.
     * @param world World this town hall belongs to.
     * @param inv Shared inventory receiving deposited resources.
     */
    TownHall(size_t id, const std::string& name, const WorldBase& world,
             std::shared_ptr<InteractiveWorldInventory> inv) :
        AgentBase(id, name, world), m_inventory(std::move(inv)) {}
    /**
     * @brief Construct a TownHall with a new private inventory.
     * @param id Unique entity id.
     * @param name Entity name.
     * @param world World this town hall belongs to.
     */
    TownHall(size_t id, const std::string& name, const WorldBase& world) :
        AgentBase(id, name, world), m_inventory(std::make_shared<InteractiveWorldInventory>()) {}

    /**
     * @brief Register a spawn associated with a resource type.
     * @param itemType Resource type produced by the spawn.
     * @param spawn Spawn to register.
     */
    void AddResourceSpawn(const ItemType& itemType, std::shared_ptr<ResourceSpawn> spawn) {
        assert(m_resourceSpawns.find(itemType) == m_resourceSpawns.end() && "Spawn already registered");
        m_resourceSpawns.insert({itemType, spawn});
    }

    /**
     * @brief Deposit delivered resources into the town hall inventory.
     * @param itemType Resource type being deposited.
     * @param amount Quantity being deposited.
     * @return true if the inventory accepted the deposit.
     */
    [[nodiscard]] bool DepositResource(const ItemType& itemType, int amount) {
        return m_inventory != nullptr && m_inventory->AddItem(itemType, amount);
    }

private:
    std::shared_ptr<InteractiveWorldInventory> m_inventory; ///< Inventory receiving deposited resources.
    std::map<ItemType, std::shared_ptr<ResourceSpawn>> m_resourceSpawns{}; ///< Optional spawn registry.

    /// @brief Town halls do not act autonomously.
    size_t SelectAction(const WorldGrid&) override { return 0; }
};
} // namespace cse498
