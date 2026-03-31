/**
 * @file PlayerAgent.hpp
 * @brief Human-controlled player (text input / interface agent).
 */

#pragma once


#include "../core/InterfaceBase.hpp"
#include "PlayerFeatures/Inventory.hpp"

namespace cse498 {

class PlayerAgent : public InterfaceBase {
private:
    Inventory mInventory;
    std::size_t mGold = 0;

public:
    PlayerAgent(size_t id, const std::string &name, const WorldBase &world);

    [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;

    [[nodiscard]] Inventory & GetInventory() { return mInventory; }
    [[nodiscard]] const Inventory& GetInventory() const { return mInventory; }

    [[nodiscard]] std::size_t GetGold() const { return mGold; }

    /**
     * Add currency directly to the player.
     */
    void AddGold(std::size_t amount) { mGold += amount; }

    /**
     * Try to spend currency.
     * @return true if enough gold was available and removed.
     */
    bool SpendGold(std::size_t amount);

    /**
     * Sets gold to a known value. Mainly useful for demos/tests.
     */
    void SetGold(std::size_t amount) { mGold = amount; }
};

}

