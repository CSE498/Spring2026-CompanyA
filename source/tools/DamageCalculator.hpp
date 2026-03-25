#pragma once

#include "CombatStats.hpp"

namespace cse498 {

  class DamageCalculator {
  public:
    [[nodiscard]] static double Calculate(const CombatStats &attacker, const CombatStats &defender);
  };

} // namespace cse498
