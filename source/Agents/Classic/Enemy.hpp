/**
 * @file Enemy.hpp
 * @author Logan Rimarcik
 *
 * Classifies an enemy agent. This can be a whole suite of possible enemies depending on features created in the
 * factory
 */

#pragma once


#include "../../core/AgentBase.hpp"
#include "core/WorldBase.hpp"
#include "tools/DamageCalculator.hpp"

namespace cse498 {
class Enemy : public AgentBase {
private:
    // Stats live in AgentBase

    std::size_t mGoldDrop = 5; // Gold amount rewarded upon defeat
    bool mGoldClaimed = false; // Whether gold has been claimed
    std::string mBuiltInWeaponName; // Intrinsic weapon identity (e.g., "Dagger")

public:
    Enemy(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {}

    /**
     * Carries out the action of the enemy -- Triggers the behavior tree and allows a turn
     * of the enemy to progress
     * @param grid - unused variable as agents have access to WorldBase --> GetGrid.
     * @return the action to be carried out -- ONLY "wasd, stay" -- Attacks handled internally
     *          (unless demo world)
     */
    [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override;


    /**
     * Sets how much currency the enemy awards when defeated.
     */
    void SetGoldDrop(std::size_t gold) { mGoldDrop = gold; }

    /**
     * Returns configured enemy drop amount without consuming it.
     */
    [[nodiscard]] std::size_t GetGoldDrop() const { return mGoldDrop; }

    /**
     * Sets this enemy's intrinsic (non-inventory) weapon label.
     * @param weaponName Display/identity name for built-in weapon behavior.
     */
    void SetBuiltInWeaponName(const std::string& weaponName) { mBuiltInWeaponName = weaponName; }

    /**
     * @return configured intrinsic weapon label, or empty string if unspecified.
     */
    [[nodiscard]] const std::string& GetBuiltInWeaponName() const { return mBuiltInWeaponName; }

    /**
     * Claims the enemy's gold reward once.
     * Subsequent calls return 0 so the reward cannot be duplicated.
     */
    std::size_t ClaimGoldDrop();

    /**
     * Resets claim state if the enemy is reused in a test/demo.
     */
    void ResetGoldDropClaim() { mGoldClaimed = false; }

    bool Interact() override {
        TakeDamage(DamageCalculator::Calculate(world.GetPlayer()->GetStats(), mStats));
        return true;
    }
};


} // namespace cse498
