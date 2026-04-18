/**
 * @file Enemy.cpp
 * @author lrima
 */

#include "Enemy.hpp"
#include "../../core/WorldBase.hpp"

namespace cse498 {
size_t Enemy::SelectAction([[maybe_unused]] const WorldGrid& grid) {

    if (!mBehaviorRoot)
        return 0;

    mBlackboard.Remove("selected_action");

    BehaviorTrees::ExecutionContext ctx(mBlackboard);
    mBehaviorRoot->Tick(ctx);

    // ***for an enemy this will only be wasd, stay***
    // REASON: passing attack codes in here doesn't define who the enemy is attacking so that is handled within
    // the behavior tree. (Enemies can attack enemies, helpers attack enemies)

    // Note: The demo tree does not do this because we don't have world functions for finding certain agents
    // in our surrounding that the world group "should" have done because we requested it.
    return mBlackboard.Get<size_t>("selected_action", 0);
}

std::size_t Enemy::ClaimGoldDrop() {
    if (mGoldClaimed) {
        return 0;
    }

    mGoldClaimed = true;
    return mGoldDrop;
}

void Enemy::TakeDamage(double amount) {
    AgentBase::TakeDamage(amount);
    world.GetAnalyticsManager()->LogRunDamage(amount);
}

void Enemy::OnDeath() { world.GetAnalyticsManager()->LogRunEnemiesKilled(1); }
} // namespace cse498
