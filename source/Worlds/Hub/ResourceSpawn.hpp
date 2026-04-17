/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents a spawn point for resources that the Agents will grab from
 * @note Status: PROPOSAL
 **/
#pragma once
#include <limits>
#include "../../core/AgentBase.hpp"
#include "ItemType.hpp"

namespace cse498 {
    /// @class Resource Spawn
    /// @brief In world spawn point
    class ResourceSpawn : public AgentBase {
    private:
        ItemType m_itemType = ItemType::Wood;
        int m_quantity = 0;

    public:
        /**
         * Constructor for Resource Spawn
         * @param id unique entity ID
         * @param name Name of entity
         * @param world world this ResourceSpawn belongs to
         * @param itemType type of item this resource pool holds
         */
        ResourceSpawn(size_t id, const std::string& name, const WorldBase& world) :
            AgentBase(id, name, world), m_itemType(ItemType::Wood) {}

        ResourceSpawn(size_t id, const std::string& name, const WorldBase& world, const ItemType& itemType) :
            AgentBase(id, name, world), m_itemType(itemType) {}

        /**
         * Get the quantity of item in the pool
         * @return
         */
        int GetQuantity() const { return m_quantity; }
        /**
         * Select action as agent. In this case do nothing
         * @return
         */
        size_t SelectAction(const WorldGrid&) override { return 0; }

        /**
         * Add resources to the pool
         * @param quantity quantity to add
         */
        void AddResource(const int& quantity) {
            // Check for integer overflow
            if (quantity > std::numeric_limits<int>::max() - m_quantity)
                return;
            m_quantity += quantity;
        }

        /**
         * Collect all resources from the resource spawn
         * @return the quantity of items in the resource spawn
         */
        [[nodiscard]] int Collect() {
            int returnQuantity = m_quantity;
            m_quantity = 0;
            return returnQuantity;
        }

        /**
         * Get the item type of the resource spawn
         * @return
         */
        ItemType GetItemType() const { return m_itemType; }
    };
} // namespace cse498
