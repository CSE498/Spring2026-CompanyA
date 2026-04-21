/**
 * @file AgentStats.hpp
 * @author Logan
 *
 *
 */

#pragma once

#include <cstddef> // for size_t

namespace cse498 {

struct AgentStats {
    /// Starting Hp
    double mMaxHp = 0;
    /// Current HP (if you want it to be lower, then make enemy take damage on spawn
    double mHp = 0; // can have negative hp
    double mAtk = 0; // can have negative atk (would heal others)
    double mDef = 0; // can have negative def. Debuffs = extra dmg
    size_t mRange = 0; // range is positive
    size_t mLevel = 0; // level >= 0
    AgentStats(double hp, double atk, double def, size_t range, size_t level) :
        mMaxHp(hp), mHp(hp), mAtk(atk), mDef(def), mRange(range), mLevel(level) {}
    AgentStats() = default;
};

} // namespace cse498
