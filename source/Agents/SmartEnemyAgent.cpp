#include "SmartEnemyAgent.hpp"

#include <array>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "../Worlds/AIWorld.hpp"

namespace cse498 {

namespace {
constexpr size_t kRecentMemory = 10;
}

bool SmartEnemyAgent::Initialize() {
  SetSymbol('S');
  return true;
}

bool SmartEnemyAgent::IsWalkable(const WorldGrid &grid, WorldPosition pos) const {
  if (!grid.IsValid(pos)) return false;
  const char tile = grid.GetCellTypeSymbol(grid[pos]);
  return tile != '#';
}

/// Need to modify this when multiple agents are
std::optional<WorldPosition> SmartEnemyAgent::GetTargetPlayerPosition() const {
  if (!GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition my_pos = GetLocation().AsWorldPosition();
  int best_dist = std::numeric_limits<int>::max();
  std::optional<WorldPosition> best_target;

  // Assumes the "player" is another agent in the world.
  for (size_t i = 0; i < world.GetNumAgents(); ++i) {
    const AgentBase &agent = world.GetAgent(i);

    if (agent.GetID() == GetID()) continue;
    if (!agent.GetLocation().IsPosition()) continue;

    const WorldPosition pos = agent.GetLocation().AsWorldPosition();
    const int dist =
        static_cast<int>(std::abs(static_cast<int>(my_pos.CellX()) - static_cast<int>(pos.CellX())) +
                         std::abs(static_cast<int>(my_pos.CellY()) - static_cast<int>(pos.CellY())));

    if (dist < best_dist) {
      best_dist = dist;
      best_target = pos;
    }
  }

  return best_target;
}

std::optional<size_t> SmartEnemyAgent::AttackActionForAdjacentPlayer() const {
  const auto target = GetTargetPlayerPosition();
  if (!target.has_value() || !GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition cur = GetLocation().AsWorldPosition();

  if (cur.Up() == *target) return GetActionID("attack_up");
  if (cur.Down() == *target) return GetActionID("attack_down");
  if (cur.Left() == *target) return GetActionID("attack_left");
  if (cur.Right() == *target) return GetActionID("attack_right");

  return std::nullopt;
}

bool SmartEnemyAgent::HasLineOfSight(
    const WorldGrid &grid,
    WorldPosition from,
    WorldPosition to) const {
  if (from.CellX() == to.CellX()) {
    const size_t x = from.CellX();
    const size_t y1 = std::min(from.CellY(), to.CellY());
    const size_t y2 = std::max(from.CellY(), to.CellY());

    for (size_t y = y1 + 1; y < y2; ++y) {
      if (!IsWalkable(grid, WorldPosition{x, y})) return false;
    }
    return true;
  }

  if (from.CellY() == to.CellY()) {
    const size_t y = from.CellY();
    const size_t x1 = std::min(from.CellX(), to.CellX());
    const size_t x2 = std::max(from.CellX(), to.CellX());

    for (size_t x = x1 + 1; x < x2; ++x) {
      if (!IsWalkable(grid, WorldPosition{x, y})) return false;
    }
    return true;
  }

  return false;
}

std::optional<size_t> SmartEnemyAgent::LineOfSightPursuitMove(
    const WorldGrid &grid,
    WorldPosition target) const {
  if (!GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition cur = GetLocation().AsWorldPosition();

  if (!HasLineOfSight(grid, cur, target)) return std::nullopt;

  if (cur.CellX() == target.CellX()) {
    if (target.CellY() < cur.CellY()) return GetActionID("up");
    if (target.CellY() > cur.CellY()) return GetActionID("down");
  }

  if (cur.CellY() == target.CellY()) {
    if (target.CellX() < cur.CellX()) return GetActionID("left");
    if (target.CellX() > cur.CellX()) return GetActionID("right");
  }

  return std::nullopt;
}

std::optional<size_t> SmartEnemyAgent::NextMoveToward(
    const WorldGrid &grid,
    WorldPosition target) const {
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world || !GetLocation().IsPosition()) return std::nullopt;

  const WorldPosition start = GetLocation().AsWorldPosition();
  if (start == target) return std::nullopt;

  std::queue<WorldPosition> frontier;
  std::unordered_map<WorldPosition, WorldPosition> parent;
  std::unordered_set<WorldPosition> visited;

  frontier.push(start);
  visited.insert(start);

  while (!frontier.empty()) {
    const WorldPosition pos = frontier.front();
    frontier.pop();

    if (pos == target) break;

    const std::array<WorldPosition, 4> neighbors{
        pos.Up(), pos.Down(), pos.Left(), pos.Right()
    };

    for (const auto &n : neighbors) {
      if (visited.contains(n)) continue;
      if (!IsWalkable(grid, n)) continue;

      // Don't path through other agents unless n is the target tile.
      if (ai_world->IsAgentAtPosition(n, GetID()) && !(n == target)) continue;

      visited.insert(n);
      parent[n] = pos;
      frontier.push(n);
    }
  }

  if (!visited.contains(target)) return std::nullopt;

  WorldPosition step = target;
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

std::optional<size_t> SmartEnemyAgent::ShortestAttackLaneMove(
    const WorldGrid &grid,
    WorldPosition target) const {
  const auto *ai_world = dynamic_cast<const AIWorld *>(&world);
  if (!ai_world) return std::nullopt;

  const std::array<WorldPosition, 4> attack_tiles{
      target.Up(), target.Down(), target.Left(), target.Right()
  };

  size_t best_distance = std::numeric_limits<size_t>::max();
  std::optional<WorldPosition> best_tile;

  for (const auto &tile : attack_tiles) {
    if (!grid.IsValid(tile)) continue;
    if (!IsWalkable(grid, tile)) continue;

    // Can't stand on another occupied agent tile unless it's our own position.
    if (ai_world->IsAgentAtPosition(tile, GetID()) && tile != GetLocation().AsWorldPosition()) {
      continue;
    }

    // Use BFS reachability by asking for a move toward that tile.
    const auto move = NextMoveToward(grid, tile);
    if (!move.has_value()) continue;

    // Full BFS distance
    std::queue<std::pair<WorldPosition, size_t>> frontier;
    std::unordered_set<WorldPosition> visited;
    const WorldPosition start = GetLocation().AsWorldPosition();

    frontier.push({start, 0});
    visited.insert(start);

    bool found = false;
    size_t dist = 0;

    while (!frontier.empty() && !found) {
      auto [pos, d] = frontier.front();
      frontier.pop();

      if (pos == tile) {
        dist = d;
        found = true;
        break;
      }

      const std::array<WorldPosition, 4> neighbors{
          pos.Up(), pos.Down(), pos.Left(), pos.Right()
      };

      for (const auto &n : neighbors) {
        if (visited.contains(n)) continue;
        if (!IsWalkable(grid, n)) continue;
        if (ai_world->IsAgentAtPosition(n, GetID()) && !(n == tile)) continue;

        visited.insert(n);
        frontier.push({n, d + 1});
      }
    }

    if (found && dist < best_distance) {
      best_distance = dist;
      best_tile = tile;
    }
  }

  if (!best_tile.has_value()) return std::nullopt;
  return NextMoveToward(grid, *best_tile);
}

std::optional<size_t> SmartEnemyAgent::ExploreMove(const WorldGrid &grid) const {
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
    if (ai_world && ai_world->IsAgentAtPosition(pos, GetID())) continue;

    size_t score = mVisitCounts.contains(pos) ? mVisitCounts.at(pos) : 0;

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

size_t SmartEnemyAgent::SelectAction(const WorldGrid &grid) {
  if (!GetLocation().IsPosition()) return 0;

  const WorldPosition current = GetLocation().AsWorldPosition();
  mVisitCounts[current] += 1;
  mRecentPositions.push_back(current);
  if (mRecentPositions.size() > kRecentMemory) {
    mRecentPositions.pop_front();
  }

  // 1. Attack immediately if adjacent.
  if (const auto attack = AttackActionForAdjacentPlayer(); attack.has_value()) {
    return *attack;
  }

  // 2. Hunt the player.
  if (const auto target = GetTargetPlayerPosition(); target.has_value()) {

    // If visible in same row/col, pursue directly.
    if (const auto los_move = LineOfSightPursuitMove(grid, *target); los_move.has_value()) {
      return *los_move;
    }

    // Otherwise move to the best adjacent attack tile.
    if (const auto lane_move = ShortestAttackLaneMove(grid, *target); lane_move.has_value()) {
      return *lane_move;
    }
  }

  // 3. Fallback exploration.
  const auto fallback = ExploreMove(grid);
  return fallback.value_or(0);
}

} // namespace cse498
