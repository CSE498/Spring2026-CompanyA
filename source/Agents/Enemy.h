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

namespace cse498
{
    class Enemy : public AgentBase
    {
    private:
        double mAttackRange = 3; // 3 block units

    public:
        Enemy(size_t id, const std::string & name, WorldBase & world) : AgentBase(id, name, world) {}

        [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;

        [[nodiscard]] double GetAttackRange() const { return mAttackRange; }
    };


}

#endif //CSE498COMPA_ENEMY_H
