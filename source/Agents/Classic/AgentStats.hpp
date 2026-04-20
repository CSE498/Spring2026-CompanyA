/**
 * @file AgentStats.hpp
 * @brief Lightweight stat container shared by player/enemy agents.
 * @author Logan
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
    double mRange = 0; // range is positive
    size_t mLevel = 0; // level >= 0
    /**
     * Construct with baseline combat values.
     * @param hp Max and initial HP.
     * @param atk Base attack value.
     * @param def Base defense value.
     * @param range Attack range in grid units.
     * @param level Agent level.
     */
    AgentStats(double hp, double atk, double def, double range, size_t level) :
        mMaxHp(hp), mHp(hp), mAtk(atk), mDef(def), mRange(range), mLevel(level)
    {
        assert(range >= 0 && "Range is positive double");
    }
    AgentStats() = default;
};

} // namespace cse498
