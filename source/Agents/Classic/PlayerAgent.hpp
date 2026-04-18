/**
 * @file PlayerAgent.hpp
 * @brief Human-controlled player (text input / interface agent).
 */

#pragma once


#include "../../core/AgentBase.hpp"
#include "PlayerFeatures/Inventory.hpp"

namespace cse498 {

class PlayerAgent : public AgentBase {
private:
    Inventory mInventory;
    /// the amount of gold the player has
    std::size_t mGold = 0;

public:
    /**
     * Constructor
     * @param id - should be 0 for the player agent in general
     * @param name - name of the agent
     * @param world - world it belongs to
     */
    PlayerAgent(size_t id, const std::string& name, const WorldBase& world);

    /**
     * Don't call this function. Needs to be overriden but doesn't serve a good use
     * @param grid
     * @return
     */
    [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override;

    /**
     * Accepts char input and returns action
     *
     * This is the entrance for the UI to tell the player what the user has clicked
     * This would need to be changed when designing 8-directional movement
     * @param input - character pressed
     */
    size_t SelectPlayerAction(char input);

    [[nodiscard]] Inventory& GetInventory() { return mInventory; }
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

} // namespace cse498
