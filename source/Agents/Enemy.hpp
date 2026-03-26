/**
 * @file Enemy.hpp
 * @author Logan Rimarcik
 *
 * Classifies an enemy agent. This can be a whole suite of possible enemies depending on features created in the
 * factory
 */

#ifndef CSE498COMPA_ENEMY_H
#define CSE498COMPA_ENEMY_H
#include "core/AgentBase.hpp"

namespace cse498
{
    class Enemy : public AgentBase
    {
    private:
        double mAttackRange = 3; // 3 block units

        std::size_t mGoldDrop = 5; // Gold amount rewarded upon defeat
        bool mGoldClaimed = false; // Whether gold has been claimed

    public:
        Enemy(size_t id, const std::string & name, WorldBase & world) : AgentBase(id, name, world) {}

        [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;

        [[nodiscard]] double GetAttackRange() const { return mAttackRange; }

        /**
         * Sets how much currency the enemy awards when defeated.
         */
        void SetGoldDrop(std::size_t gold) { mGoldDrop = gold; }

        /**
         * Returns configured enemy drop amount without consuming it.
         */
        [[nodiscard]] std::size_t GetGoldDrop() const { return mGoldDrop; }

        /**
         * Claims the enemy's gold reward once.
         * Subsequent calls return 0 so the reward cannot be duplicated.
         */
        std::size_t ClaimGoldDrop();

        /**
         * Resets claim state if the enemy is reused in a test/demo.
         */
        void ResetGoldDropClaim() { mGoldClaimed = false; }

    };


}

#endif //CSE498COMPA_ENEMY_H
