/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Resource node that accumulates resources for fetch agents.
 **/
#pragma once
#include <limits>
#include "../../core/AgentBase.hpp"
#include "ItemType.hpp"

namespace cse498 {
/**
 * @class ResourceSpawn
 * @brief In-world resource node for one ItemType.
 *
 * ResourceSpawn stores raw resources produced by a ResourceProducer. The GUI
 * uses quantity thresholds to render generated empty, partial, and full states.
 * FetchAgent's default pickup waits until the spawn reaches the full state
 * before collecting.
 */
class ResourceSpawn : public AgentBase {
private:
    static constexpr int DEFAULT_MAX_COLLECTION_QUANTITY = 10; ///< Default pickup cap per collection.

    ItemType m_itemType = ItemType::Wood; ///< Resource type stored by this spawn.
    int m_quantity = 0; ///< Current amount waiting in the spawn.
    int m_maxCollectionQuantity = DEFAULT_MAX_COLLECTION_QUANTITY; ///< Maximum amount collected per pickup.

public:
    /**
     * @brief Construct a wood ResourceSpawn.
     * @param id Unique entity ID.
     * @param name Name of entity.
     * @param world World this ResourceSpawn belongs to.
     */
    ResourceSpawn(size_t id, const std::string& name, const WorldBase& world) :
        AgentBase(id, name, world), m_itemType(ItemType::Wood) {}

    /**
     * @brief Construct a ResourceSpawn for a specific resource type.
     * @param id Unique entity ID.
     * @param name Name of entity.
     * @param world World this ResourceSpawn belongs to.
     * @param itemType Type of item this resource pool holds.
     */
    ResourceSpawn(size_t id, const std::string& name, const WorldBase& world, const ItemType& itemType) :
        AgentBase(id, name, world), m_itemType(itemType) {}

    /**
     * @brief Get the current quantity stored at this spawn.
     * @return Amount of resource available.
     */
    int GetQuantity() const { return m_quantity; }
    /**
     * @brief Resource spawns do not act autonomously.
     * @return Remain-still action id.
     */
    size_t SelectAction(const WorldGrid&) override { return 0; }

    /**
     * @brief Add resources to the pool.
     * @param quantity Quantity to add. Non-positive and overflowing adds are ignored.
     */
    void AddResource(const int& quantity) {
        if (quantity <= 0) {
            return;
        }
        // Check for integer overflow
        if (quantity > std::numeric_limits<int>::max() - m_quantity)
            return;
        m_quantity += quantity;
    }
    /**
     * @brief Set the maximum quantity collected by one pickup.
     *
     * InteractiveWorld uses twice this amount as the full-state threshold for
     * generated resource visuals and default FetchAgent pickup readiness.
     *
     * @param maxQuant New maximum collection quantity.
     */
    void SetMaxCollectionQuantity(int maxQuant) {
        if (maxQuant > 0) {
            m_maxCollectionQuantity = maxQuant;
        }
    }
    /**
     * @brief Get the maximum quantity collected by one pickup.
     * @return Current maximum collection quantity.
     */
    [[nodiscard]] int GetMaxCollectionQuantity() const { return m_maxCollectionQuantity; }
    /**
     * @brief Collect resources from the spawn, capped by GetMaxCollectionQuantity().
     * @return Quantity collected.
     */
    [[nodiscard]] int Collect() {
        int collectAmount = std::min(m_quantity, m_maxCollectionQuantity);
        m_quantity -= collectAmount;
        return collectAmount;
    }

    /**
     * @brief Get the item type stored by this spawn.
     * @return Resource type.
     */
    ItemType GetItemType() const { return m_itemType; }
};
} // namespace cse498
