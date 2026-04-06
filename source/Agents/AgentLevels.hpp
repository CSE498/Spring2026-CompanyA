/**
 * @file AgentLevels.hpp
 * @author lrima
 *
 * Defines leveling for agents for easier stat creation
 */

#pragma once
#include "core/AgentBase.hpp"

namespace cse498
{
// stat object structure to be owned by the enemy object


class AgentLevels
{
public:
    // Example of how skeleton stats are defined and made
    /**
     * Gets the stat object scaled by some leveling of the skeleton
     * @param level - level of the skeleton
     * @return stat object
     */
    static constexpr AgentStats GetSkeletonStats(size_t level)
    {
        double hp = 100 + 20 * level;
        double atk = 5 + 2 * level;
        double def = 5 + level;
        size_t range = 3 + static_cast<int>(level / 5);
        return {hp, atk, def, range, level};
    }




};

}




