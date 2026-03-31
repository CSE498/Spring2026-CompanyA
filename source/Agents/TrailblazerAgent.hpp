/**
 * @file TrailblazerAgent.hpp
 * @brief Goal-oriented agent: heal when hurt, collect weapon/heal
 *        loot, attack adjacent enemies, otherwise explore.
 *
 * @details Uses AIWorld APIs (items, enemies); run inside
 *          AIWorld for full behavior. See docs/Group1.md for requirements and
 *          comparison with EnemyAgent.
 */
#pragma once

#include <deque>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../core/AgentBase.hpp"

namespace cse498 {

/// GOAP-style planner over combat/loot goals; designed for AIWorld.
class TrailblazerAgent : public AgentBase {
public:
  enum class PlanAction {
    MoveToHeal,
    PickUpHeal,
    UseHeal,
    MoveToWeapon,
    PickUpWeapon,
    MoveToEnemy,
    AttackEnemy,
    Explore
  };

private:
  std::unordered_map<WorldPosition, size_t> mVisitCounts;
  std::deque<WorldPosition> mRecentPositions;

  [[nodiscard]] bool IsWalkable(const WorldGrid &grid, WorldPosition pos) const;
  [[nodiscard]] std::optional<WorldPosition>
  NearestItemPosition(bool heal_item) const;
  [[nodiscard]] std::optional<WorldPosition> NearestEnemyPosition() const;
  [[nodiscard]] std::optional<size_t>
  NextMoveToward(const WorldGrid &grid, WorldPosition target,
                 bool allow_enemy_target) const;
  [[nodiscard]] std::optional<size_t>
  ChaseEnemyMove(const WorldGrid &grid, WorldPosition enemy_pos) const;
  [[nodiscard]] std::optional<size_t> ExploreMove(const WorldGrid &grid) const;
  [[nodiscard]] std::optional<size_t> AttackActionForAdjacentEnemy() const;
  [[nodiscard]] std::vector<PlanAction> BuildPlan(const WorldGrid &grid) const;
  [[nodiscard]] std::optional<size_t>
  ExecutePlanAction(const WorldGrid &grid, PlanAction action) const;

public:
  TrailblazerAgent(size_t id, const std::string &name, const WorldBase &world)
      : AgentBase(id, name, world) {}

  bool Initialize() override;
  [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;
};

} // namespace cse498
