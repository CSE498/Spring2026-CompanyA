#include "DamageCalculator.hpp"

#include <algorithm>

namespace cse498 {

  double DamageCalculator::Calculate(const CombatStats &attacker, const CombatStats &defender) {
    return std::max(1.0, attacker.attack - defender.defense);
  }

} // namespace cse498
