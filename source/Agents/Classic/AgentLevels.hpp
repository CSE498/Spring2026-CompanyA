/**
 * @file AgentLevels.hpp
 * @author lrima
 *
 * Defines leveling for agents for easier stat creation
 */

#pragma once
#include "../../core/AgentBase.hpp"

namespace cse498 {
// stat object structure to be owned by the enemy object


class AgentLevels {
public:
    // Example of how skeleton stats are defined and made
    /**
     * Gets the stat object scaled by some leveling of the skeleton
     * @param level - level of the skeleton
     * @return stat object
     */
    static constexpr AgentStats GetSkeletonStats(size_t level) {
        double hp = 100 + 20 * static_cast<double>(level);
        double atk = 5 + 2 * static_cast<double>(level);
        double def = 5 + static_cast<double>(level);
        size_t range = 3 + level / 5;
        return {hp, atk, def, range, level};
    }

    /**
     * Gets level-scaled goblin stats.
     * Goblins are frailer but hit harder at shorter range.
     * @param level - level of the goblin
     * @return stat object
     */
    static constexpr AgentStats GetGoblinStats(size_t level) {
        double hp = 80 + 16 * static_cast<double>(level);
        double atk = 8 + 3 * static_cast<double>(level);
        double def = 4 + static_cast<double>(level);
        size_t range = 2 + level / 6;
        return {hp, atk, def, range, level};
    }
};

} // namespace cse498
