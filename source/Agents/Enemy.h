/**
 * @file Enemy.h
 * @author Logan Rimarcik
 *
 * Classifies an enemy agent. This can be a whole suite of possible enemies depending on features created in the
 * factory
 */

#ifndef CSE498COMPA_ENEMY_H
#define CSE498COMPA_ENEMY_H
#include "core/AgentBase.hpp"
#include "tools/BehaviorTree/BehaviorTree.hpp"

namespace cse498
{
    class Enemy : public AgentBase
    {
    private:
        /// can be stored inside a more compact structure -- modify when bringing in more stats
        /// range of the enemy
        double mAttackRange = 3; // 3 block units

        /// behavior tree root
        std::unique_ptr<BehaviorTrees::Node> mBehaviorRoot = nullptr;

    public:
        /**
         *
         * @param id
         * @param name
         * @param world  - reference to the world since we change the index of the agent id counter.
         *                  could make this const and make that function const mutable.
         */
        Enemy(size_t id, const std::string & name, WorldBase & world) : AgentBase(id, name, world) {}

        void SetBehaviorTree(std::unique_ptr<BehaviorTrees::Node> root) { mBehaviorRoot = std::move(root);}

        [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;

        [[nodiscard]] double GetAttackRange() const { return mAttackRange; }
    };


}

#endif //CSE498COMPA_ENEMY_H
