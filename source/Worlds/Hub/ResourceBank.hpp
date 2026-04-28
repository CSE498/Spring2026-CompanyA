/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief Simple resource storage building for the interactive world.
 * @note Status: PROPOSAL
 **/

#pragma once

#include "../../core/AgentBase.hpp"
#include "InteractiveWorldInventory.hpp"
#include "ItemType.hpp"

namespace cse498 {

/**
 * @class ResourceBank
 * @brief Stores resources locally so other agents can deposit to and withdraw
 *        from it.
 *
 * This class is intended to act as an intermediate storage endpoint in hauling
 * pipelines, such as ResourceSpawn -> ResourceBank -> TownHall.
 */
class ResourceBank : public AgentBase {
public:
    using ItemCount = InteractiveWorldInventory::ItemCount;

    ResourceBank(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {}

    /**
     * Deposit resources into the bank.
     *
     * @param itemType Type of resource being deposited
     * @param amount Quantity to deposit
     * @return true if the deposit succeeded
     */
    bool DepositResource(ItemType itemType, int amount) {
        if (amount < 0) {
            return false;
        }

        return m_inventory.AddItem(itemType, static_cast<ItemCount>(amount));
    }

    /**
     * Withdraw resources from the bank.
     *
     * @param itemType Type of resource being withdrawn
     * @param amount Quantity to withdraw
     * @return true if the withdrawal succeeded
     */
    bool WithdrawResource(ItemType itemType, int amount) {
        if (amount < 0) {
            return false;
        }

        return m_inventory.RemoveItem(itemType, static_cast<ItemCount>(amount));
    }

    /**
     * @return Current stored amount for a resource type
     */
    [[nodiscard]] ItemCount GetStoredAmount(ItemType itemType) const { return m_inventory.GetAmount(itemType); }

    /**
     * @return true if at least `amount` of the resource is available
     */
    [[nodiscard]] bool HasEnough(ItemType itemType, ItemCount amount) const {
        return m_inventory.HasEnough(itemType, amount);
    }

    /**
     * @return Read-only access to the bank's inventory
     */
    [[nodiscard]] const InteractiveWorldInventory& GetOverworldInventory() const { return m_inventory; }

    size_t SelectAction(const WorldGrid&) override { return 0; }

private:
    InteractiveWorldInventory m_inventory;
};

} // namespace cse498
