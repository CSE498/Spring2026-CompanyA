/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Class representing the town hall for interactive world
 * @note Status: PROPOSAL
 **/

#pragma once

#include "../../core/AgentBase.hpp"
#include "InteractiveWorldInventory.hpp"
#include "ItemType.hpp"
#include "ResourceSpawn.hpp"
#include "../../Agents/PacingAgent.hpp"

#include <vector>
#include <map>

namespace cse498
{
/// @class Building
/// @brief Town hall building. Stores the world inventory and responsible for managing fetch agents
class TownHall : public AgentBase
{
public:
    TownHall(size_t id, const std::string& name, const WorldBase& world, std::shared_ptr<InteractiveWorldInventory> inv)
	: AgentBase(id, name, world), m_inventory(std::move(inv)) {}
    TownHall(size_t id, const std::string& name, const WorldBase& world)
	: AgentBase(id, name, world), m_inventory(nullptr) {}

    void AddResourceSpawn(const ItemType& itemType, std::shared_ptr<ResourceSpawn> spawn)
    {
	    assert(m_resourceSpawns.find(itemType) == m_resourceSpawns.end() && "Spawn already registered");
	    m_resourceSpawns.insert({ itemType, spawn });
    }

    void DepositResource(const ItemType& itemType, int amount)
    {
	    m_inventory->AddItem(itemType, amount);
    }

    // TODO: Create fetch agents and add them to world
    //void BuyFetchAgent(const ItemType& itemType)
    //{

   // }
private:
    std::shared_ptr<InteractiveWorldInventory> m_inventory;
    std::map<ItemType, std::shared_ptr<ResourceSpawn>> m_resourceSpawns {};

    size_t SelectAction(const WorldGrid&) override { return 0; }
};
}