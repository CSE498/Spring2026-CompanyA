#include "TrailblazerAgent.hpp"

#include <array>
#include <deque>
#include <limits>
#include <queue>
#include <unordered_set>

#include "../Worlds/AIWorld.hpp"

namespace cse498 {

namespace {
constexpr size_t kRecentMemory = 10;
constexpr int kLowHpThreshold = 4;

enum Predicate : uint32_t {
  LOW_HP         = 1u << 0,
  HAS_HEAL       = 1u << 1,
  HAS_WEAPON     = 1u << 2,
  AT_HEAL_ITEM   = 1u << 3,
  AT_WEAPON_ITEM = 1u << 4,
  ENEMY_ADJ      = 1u << 5,
  ENEMY_ALIVE    = 1u << 6,
  HEAL_VISIBLE   = 1u << 7,
  WEAPON_VISIBLE = 1u << 8
};

struct GoapActionDef {
  TrailblazerAgent::PlanAction action;
  uint32_t required_mask;
  uint32_t required_value;
  uint32_t clear_mask;
  uint32_t set_mask;
  size_t cost;
};

bool MeetsGoal(uint32_t state, uint32_t goal_mask, uint32_t goal_value) {
  return (state & goal_mask) == goal_value;
}
}

bool TrailblazerAgent::Initialize() {
  SetSymbol('T');
  return true;
}

bool TrailblazerAgent::IsWalkable(const WorldGrid &grid, WorldPosition pos) const {
  if (!grid.IsValid(pos)) return false;
  const char tile = grid.GetCellTypeSymbol(grid[pos]);
  return tile != '#';
}

std::optional<WorldPosition> TrailblazerAgent::NearestItemPosition(bool heal_item) const {
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world || !GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition current = GetLocation().AsWorldPosition();
  const auto items = ai_world->GetFloorItems();
  int best_dist = std::numeric_limits<int>::max();
  std::optional<WorldPosition> best;

  for (const auto &item : items) {
    const bool is_heal = (item.mKind == LootItem::ItemKind::Heal);
    if (is_heal != heal_item) continue;
    const int dist = static_cast<int>(
        std::abs(static_cast<int>(item.mPosition.CellX()) - static_cast<int>(current.CellX())) +
        std::abs(static_cast<int>(item.mPosition.CellY()) - static_cast<int>(current.CellY())));
    if (dist < best_dist) {
      best_dist = dist;
      best = item.mPosition;
    }
  }
  return best;
}

std::optional<WorldPosition> TrailblazerAgent::NearestEnemyPosition() const {
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world || !GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition current = GetLocation().AsWorldPosition();
  const auto &enemies = ai_world->GetEnemies();
  int best_dist = std::numeric_limits<int>::max();
  std::optional<WorldPosition> best;

  for (const auto &enemy : enemies) {
    if (!enemy.mAlive) continue;
    const int dist = static_cast<int>(
        std::abs(static_cast<int>(enemy.mPosition.CellX()) - static_cast<int>(current.CellX())) +
        std::abs(static_cast<int>(enemy.mPosition.CellY()) - static_cast<int>(current.CellY())));
    if (dist < best_dist) {
      best_dist = dist;
      best = enemy.mPosition;
    }
  }
  return best;
}

std::optional<size_t> TrailblazerAgent::AttackActionForAdjacentEnemy() const {
  const auto * ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world || !GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition current = GetLocation().AsWorldPosition();
  if (ai_world->IsEnemyAtPosition(current.Up())) return GetActionID("attack_up");
  if (ai_world->IsEnemyAtPosition(current.Down())) return GetActionID("attack_down");
  if (ai_world->IsEnemyAtPosition(current.Left())) return GetActionID("attack_left");
  if (ai_world->IsEnemyAtPosition(current.Right())) return GetActionID("attack_right");
  return std::nullopt;
}

std::optional<size_t> TrailblazerAgent::NextMoveToward(
    const WorldGrid &grid,
    WorldPosition target,
    bool allow_enemy_target) const {
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world || !GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition start = GetLocation().AsWorldPosition();
  if (start == target) return std::nullopt;

  std::queue<WorldPosition> frontier;
  std::unordered_map<WorldPosition, WorldPosition> parent;
  std::unordered_set<WorldPosition> visited;
  frontier.push(start);
  visited.insert(start);

  const auto is_walkable_for_path = [&](WorldPosition pos) {
    if (!IsWalkable(grid, pos)) return false;
    if (!allow_enemy_target && ai_world->IsEnemyAtPosition(pos)) return false;
    return true;
  };

  while (!frontier.empty()) {
    const WorldPosition pos = frontier.front();
    frontier.pop();

    if (pos == target) break;

    const std::array<WorldPosition, 4> neighbors{pos.Up(), pos.Down(), pos.Left(), pos.Right()};
    for (const auto &neighbor : neighbors) {
      if (visited.contains(neighbor)) continue;
      if (!is_walkable_for_path(neighbor)) continue;
      visited.insert(neighbor);
      parent[neighbor] = pos;
      frontier.push(neighbor);
    }
  }

  if (!visited.contains(target)) return std::nullopt;
  WorldPosition step = target;
  while (parent.contains(step) && !(parent[step] == start)) {
    step = parent[step];
  }
  if (!parent.contains(step) && !(step == target)) return std::nullopt;

  const long dx = static_cast<long>(step.CellX()) - static_cast<long>(start.CellX());
  const long dy = static_cast<long>(step.CellY()) - static_cast<long>(start.CellY());
  if (dx == 1 && dy == 0) return GetActionID("right");
  if (dx == -1 && dy == 0) return GetActionID("left");
  if (dx == 0 && dy == 1) return GetActionID("down");
  if (dx == 0 && dy == -1) return GetActionID("up");
  return std::nullopt;
}

std::optional<size_t> TrailblazerAgent::ExploreMove(const WorldGrid &grid) const {
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!GetLocation().IsPosition()) return std::nullopt;
  const WorldPosition current = GetLocation().AsWorldPosition();
  const std::array<std::pair<WorldPosition, size_t>, 4> options{{
      {current.Up(), GetActionID("up")},
      {current.Down(), GetActionID("down")},
      {current.Left(), GetActionID("left")},
      {current.Right(), GetActionID("right")}
  }};

  size_t best_action = 0;
  size_t best_score = std::numeric_limits<size_t>::max();
  for (const auto &[pos, action] : options) {
    if (!IsWalkable(grid, pos)) continue;
    if (ai_world && ai_world->IsEnemyAtPosition(pos)) continue;
    size_t score = mVisitCounts.contains(pos) ? mVisitCounts.at(pos) : 0;
    // Penalize immediate backtracking to reduce 2-cell oscillations.
    if (!mRecentPositions.empty() && pos == mRecentPositions.back()) {
      score += 100;
    }
    if (score < best_score) {
      best_score = score;
      best_action = action;
    }
  }
  if (best_action == 0) return std::nullopt;
  return best_action;
}

std::optional<size_t> TrailblazerAgent::ChaseEnemyMove(
    const WorldGrid &grid,
    WorldPosition enemy_pos) const {
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world || !GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition start = GetLocation().AsWorldPosition();
  std::queue<WorldPosition> frontier;
  std::unordered_map<WorldPosition, WorldPosition> parent;
  std::unordered_set<WorldPosition> visited;
  frontier.push(start);
  visited.insert(start);

  int best_enemy_dist = std::numeric_limits<int>::max();
  std::optional<WorldPosition> best_reachable = std::nullopt;

  while (!frontier.empty()) {
    const WorldPosition pos = frontier.front();
    frontier.pop();

    const int d = static_cast<int>(
        std::abs(static_cast<int>(pos.CellX()) - static_cast<int>(enemy_pos.CellX())) +
        std::abs(static_cast<int>(pos.CellY()) - static_cast<int>(enemy_pos.CellY())));
    if (d < best_enemy_dist) {
      best_enemy_dist = d;
      best_reachable = pos;
    }

    const std::array<WorldPosition, 4> neighbors{pos.Up(), pos.Down(), pos.Left(), pos.Right()};
    for (const auto &n : neighbors) {
      if (visited.contains(n)) continue;
      if (!IsWalkable(grid, n)) continue;
      if (ai_world->IsEnemyAtPosition(n)) continue;
      visited.insert(n);
      parent[n] = pos;
      frontier.push(n);
    }
  }

  if (!best_reachable.has_value() || *best_reachable == start) return std::nullopt;
  WorldPosition step = *best_reachable;
  while (parent.contains(step) && !(parent[step] == start)) {
    step = parent[step];
  }

  const long dx = static_cast<long>(step.CellX()) - static_cast<long>(start.CellX());
  const long dy = static_cast<long>(step.CellY()) - static_cast<long>(start.CellY());
  if (dx == 1 && dy == 0) return GetActionID("right");
  if (dx == -1 && dy == 0) return GetActionID("left");
  if (dx == 0 && dy == 1) return GetActionID("down");
  if (dx == 0 && dy == -1) return GetActionID("up");
  return std::nullopt;
}

std::vector<TrailblazerAgent::PlanAction> TrailblazerAgent::BuildPlan(const WorldGrid &grid) const {
  (void) grid;
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world || !GetLocation().IsPosition()) return {PlanAction::Explore};

  const WorldPosition current = GetLocation().AsWorldPosition();
  const auto &my_state = ai_world->GetAgentState(GetID());
  const auto items = ai_world->GetFloorItems();
  const auto enemies = ai_world->GetEnemies();

  bool at_heal_item = false;
  bool at_weapon_item = false;
  bool heal_visible = false;
  bool weapon_visible = false;
  for (const auto &item : items) {
    if (item.mKind == LootItem::ItemKind::Heal) heal_visible = true;
    if (item.mKind == LootItem::ItemKind::Weapon) weapon_visible = true;
    if (item.mPosition == current && item.mKind == LootItem::ItemKind::Heal) at_heal_item = true;
    if (item.mPosition == current && item.mKind == LootItem::ItemKind::Weapon) at_weapon_item = true;
  }

  bool enemy_alive = false;
  for (const auto &enemy : enemies) {
    if (enemy.mAlive) {
      enemy_alive = true;
      break;
    }
  }

  uint32_t start = 0;
  if (my_state.mHP <= kLowHpThreshold) start |= LOW_HP;
  if (my_state.mHealCharges > 0) start |= HAS_HEAL;
  if (my_state.mAttack > 2) start |= HAS_WEAPON;
  if (at_heal_item) start |= AT_HEAL_ITEM;
  if (at_weapon_item) start |= AT_WEAPON_ITEM;
  if (AttackActionForAdjacentEnemy().has_value()) start |= ENEMY_ADJ;
  if (enemy_alive) start |= ENEMY_ALIVE;
  if (heal_visible) start |= HEAL_VISIBLE;
  if (weapon_visible) start |= WEAPON_VISIBLE;

  uint32_t goal_mask = 0;
  uint32_t goal_value = 0;
  if (start & LOW_HP) {
    goal_mask = LOW_HP;
    goal_value = 0;
  } else if (!(start & HAS_WEAPON) && (start & WEAPON_VISIBLE)) {
    goal_mask = HAS_WEAPON;
    goal_value = HAS_WEAPON;
  } else if (start & ENEMY_ALIVE) {
    goal_mask = ENEMY_ALIVE;
    goal_value = 0;
  } else {
    return {PlanAction::Explore};
  }

  const std::vector<GoapActionDef> actions{
      {PlanAction::MoveToHeal,   HEAL_VISIBLE, HEAL_VISIBLE, 0,                AT_HEAL_ITEM,   2},
      {PlanAction::PickUpHeal,   AT_HEAL_ITEM, AT_HEAL_ITEM, AT_HEAL_ITEM,     HAS_HEAL,       1},
      {PlanAction::UseHeal,      HAS_HEAL | LOW_HP, HAS_HEAL | LOW_HP, LOW_HP | HAS_HEAL, 0,   1},
      {PlanAction::MoveToWeapon, WEAPON_VISIBLE, WEAPON_VISIBLE, 0,             AT_WEAPON_ITEM, 2},
      {PlanAction::PickUpWeapon, AT_WEAPON_ITEM, AT_WEAPON_ITEM, AT_WEAPON_ITEM, HAS_WEAPON,    1},
      {PlanAction::MoveToEnemy,  ENEMY_ALIVE, ENEMY_ALIVE, 0,                   ENEMY_ADJ,      2},
      {PlanAction::AttackEnemy,  ENEMY_ADJ | ENEMY_ALIVE, ENEMY_ADJ | ENEMY_ALIVE, ENEMY_ALIVE, 0, 1},
      {PlanAction::Explore,      0, 0, 0,                                     0,              3}
  };

  struct Node {
    uint32_t state;
    size_t cost;
    std::vector<PlanAction> plan;
  };

  std::deque<Node> frontier;
  std::unordered_map<uint32_t, size_t> best_cost;
  frontier.push_back({start, 0, {}});
  best_cost[start] = 0;

  while (!frontier.empty()) {
    Node node = frontier.front();
    frontier.pop_front();

    if (MeetsGoal(node.state, goal_mask, goal_value)) return node.plan;
    if (node.plan.size() >= 5) continue;

    for (const auto &action : actions) {
      if ((node.state & action.required_mask) != action.required_value) continue;
      uint32_t next_state = node.state;
      next_state &= ~action.clear_mask;
      next_state |= action.set_mask;

      const size_t next_cost = node.cost + action.cost;
      if (best_cost.contains(next_state) && best_cost[next_state] <= next_cost) continue;
      best_cost[next_state] = next_cost;

      Node next = node;
      next.state = next_state;
      next.cost = next_cost;
      next.plan.push_back(action.action);
      frontier.push_back(std::move(next));
    }
  }

  return {PlanAction::Explore};
}

std::optional<size_t> TrailblazerAgent::ExecutePlanAction(
    const WorldGrid &grid,
    PlanAction action) const {
  switch (action) {
  case PlanAction::PickUpHeal:
  case PlanAction::PickUpWeapon:
    return GetActionID("pickup");
  case PlanAction::UseHeal:
    return GetActionID("use_heal");
  case PlanAction::AttackEnemy: {
    const auto attack = AttackActionForAdjacentEnemy();
    if (attack.has_value()) return *attack;
    return std::nullopt;
  }
  case PlanAction::MoveToHeal: {
    const auto target = NearestItemPosition(true);
    if (!target.has_value()) return std::nullopt;
    return NextMoveToward(grid, *target, false);
  }
  case PlanAction::MoveToWeapon: {
    const auto target = NearestItemPosition(false);
    if (!target.has_value()) return std::nullopt;
    return NextMoveToward(grid, *target, false);
  }
  case PlanAction::MoveToEnemy: {
    const auto enemy_pos = NearestEnemyPosition();
    if (!enemy_pos.has_value()) return std::nullopt;
    return ChaseEnemyMove(grid, *enemy_pos);
  }
  case PlanAction::Explore:
    return ExploreMove(grid);
  }
  return std::nullopt;
}

size_t TrailblazerAgent::SelectAction(const WorldGrid &grid) {
  if (!GetLocation().IsPosition()) return 0;
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world) return 0;
  const WorldPosition current = GetLocation().AsWorldPosition();

  // Tactical reflexes before planning: fight/pickup opportunities now.
  if (const auto attack = AttackActionForAdjacentEnemy(); attack.has_value()) {
    return *attack;
  }
  for (const auto &item : ai_world->GetFloorItems()) {
    if (item.mPosition == current) return GetActionID("pickup");
  }

  mVisitCounts[current] += 1;
  mRecentPositions.push_back(current);
  if (mRecentPositions.size() > kRecentMemory) {
    mRecentPositions.pop_front();
  }

  const std::vector<PlanAction> plan = BuildPlan(grid);
  if (!plan.empty()) {
    const auto action = ExecutePlanAction(grid, plan.front());
    if (action.has_value() && *action != 0) return *action;
  }

  const auto fallback = ExploreMove(grid);
  return fallback.value_or(0);
}

} // namespace cse498
