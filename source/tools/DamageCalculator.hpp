/**
 * @file DamageCalculator.hpp
 * @brief Static helper to compute melee-style damage from attacker and defender stats.
 */
#pragma once

#include <algorithm>
#include "../Agents/Classic/AgentStats.hpp"

namespace cse498 {

/**
 * @brief Stateless damage computation using attack minus defense, floored at 1.
 */
class DamageCalculator {
public:
    /**
     * @brief Damage from @p attacker to @p defender (at least 1).
     * @param attacker Stats providing attack.
     * @param defender Stats providing defense.
     * @return `max(1, attacker.mAtk - defender.mDef)`.
     */
    [[nodiscard]] static constexpr double Calculate(const AgentStats& attacker, const AgentStats& defender) {
        return std::max(1.0, attacker.mAtk - defender.mDef);
    }
};
} // namespace cse498
