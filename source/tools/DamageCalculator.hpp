#pragma once

#include "../Agents/Classic/AgentStats.hpp"
#include <algorithm>

namespace cse498
{
class DamageCalculator
{
public:
    /**
     * Function to find the amount of damage needing to be dealt from attacker to defender
     * Does at least 1hp damage
     * @param attacker - uses attack
     * @param defender - uses defense
     * @return damage value calculated
     */
    [[nodiscard]] static constexpr double Calculate(const AgentStats &attacker, const AgentStats &defender)
    {
        return std::max(1.0, attacker.mAtk - defender.mDef);
    }
};
} // namespace cse498
