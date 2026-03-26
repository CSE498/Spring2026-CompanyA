#include "LearningExplorerAgent.hpp"

#include "../core/WorldBase.hpp"

#include <cmath>
#include <limits>
#include <queue>
#include <unordered_set>

namespace cse498 {

LearningExplorerAgent::LearningExplorerAgent(size_t id, const std::string &name,
                                             const WorldBase &world)
    : AgentBase(id, name, world) {}

bool LearningExplorerAgent::Initialize() {
  return HasAction("up") && HasAction("down") && HasAction("left") &&
         HasAction("right");
}

size_t LearningExplorerAgent::CellIndex(const WorldGrid &grid,
                                        WorldPosition pos) const {
  return pos.CellY() * grid.GetWidth() + pos.CellX();
}

int LearningExplorerAgent::GetVisitCount(const WorldGrid &grid,
                                         WorldPosition pos) const {
  const size_t index = CellIndex(grid, pos);
  auto it = visit_count_by_cell.find(index);
  return (it == visit_count_by_cell.end()) ? 0 : it->second;
}

WorldPosition LearningExplorerAgent::PredictMove(WorldPosition pos,
                                                 size_t action_id) const {
  if (action_id == GetActionID("up"))
    return pos.Up();
  if (action_id == GetActionID("down"))
    return pos.Down();
  if (action_id == GetActionID("left"))
    return pos.Left();
  if (action_id == GetActionID("right"))
    return pos.Right();
  return pos;
}

void LearningExplorerAgent::UpdateMemory(const WorldGrid &grid) {
  const WorldPosition current_pos = GetLocation().AsWorldPosition();
  visit_count_by_cell[CellIndex(grid, current_pos)]++;

  if (first_turn) {
    first_turn = false;
  }
}

WorldPosition LearningExplorerAgent::BFSNextStep(const WorldGrid &grid) const {
  const WorldPosition start = GetLocation().AsWorldPosition();
  const size_t start_idx = CellIndex(grid, start);
  const size_t wall_id = grid.GetCellTypeID("wall");

  // Collect cells occupied by other agents so we don't target them.
  std::unordered_set<size_t> agent_cells;
  const auto known = world.GetKnownAgents(*this);
  for (size_t aid : known) {
    if (aid == GetID())
      continue;
    const AgentBase &other = world.GetAgent(aid);
    if (!other.GetLocation().IsPosition())
      continue;
    agent_cells.insert(CellIndex(grid, other.GetLocation().AsWorldPosition()));
  }

  std::queue<WorldPosition> frontier;
  std::unordered_map<size_t, size_t> parent;
  parent[start_idx] = start_idx;
  frontier.push(start);

  while (!frontier.empty()) {
    WorldPosition current = frontier.front();
    frontier.pop();

    size_t cur_idx = CellIndex(grid, current);

    // Target must be unvisited AND not occupied by another agent.
    if (cur_idx != start_idx) {
      auto it = visit_count_by_cell.find(cur_idx);
      if (it == visit_count_by_cell.end() &&
          agent_cells.find(cur_idx) == agent_cells.end()) {
        size_t trace = cur_idx;
        while (parent[trace] != start_idx) {
          trace = parent[trace];
        }
        size_t tx = trace % grid.GetWidth();
        size_t ty = trace / grid.GetWidth();
        return WorldPosition{static_cast<double>(tx), static_cast<double>(ty)};
      }
    }

    const WorldPosition neighbors[] = {current.Up(), current.Down(),
                                       current.Left(), current.Right()};
    for (const auto &next : neighbors) {
      if (!grid.IsValid(next))
        continue;
      size_t next_idx = CellIndex(grid, next);
      if (wall_id != 0 && grid[next] == wall_id)
        continue;
      if (parent.find(next_idx) != parent.end())
        continue;
      parent[next_idx] = cur_idx;
      frontier.push(next);
    }
  }

  return start;
}

double
LearningExplorerAgent::ScoreAction(const WorldGrid &grid, size_t action_id,
                                   const WorldPosition &bfs_target) const {
  const WorldPosition current_pos = GetLocation().AsWorldPosition();
  const WorldPosition next_pos = PredictMove(current_pos, action_id);

  if (!grid.IsValid(next_pos))
    return -1'000'000.0;

  const size_t wall_id = grid.GetCellTypeID("wall");
  if (wall_id != 0 && grid[next_pos] == wall_id)
    return -1'000'000.0;

  double score = 0.0;

  // Avoid other agents: heavy penalty for collision, moderate for proximity.
  const auto agent_ids = world.GetKnownAgents(*this);
  for (size_t aid : agent_ids) {
    if (aid == GetID())
      continue;
    const AgentBase &other = world.GetAgent(aid);
    if (!other.GetLocation().IsPosition())
      continue;
    const WorldPosition apos = other.GetLocation().AsWorldPosition();
    const int dx = static_cast<int>(next_pos.CellX()) -
                   static_cast<int>(apos.CellX());
    const int dy = static_cast<int>(next_pos.CellY()) -
                   static_cast<int>(apos.CellY());
    const int dist = std::abs(dx) + std::abs(dy);
    if (dist == 0)
      return -1'000'000.0;
    if (dist <= 2)
      score -= 50.0;
  }

  // Strongly prefer unvisited cells; penalise revisits proportionally.
  const int visits = GetVisitCount(grid, next_pos);
  if (visits == 0) {
    score += 100.0;
  } else {
    score -= static_cast<double>(visits) * 5.0;
  }

  // BFS guidance: bonus for the move that leads toward nearest unvisited cell.
  if (!(bfs_target == current_pos) && next_pos == bfs_target) {
    score += 200.0;
  }

  // Anti-oscillation: penalize going back to the cell we just came from.
  if (has_prev_position && next_pos == prev_position) {
    score -= 30.0;
  }

  return score;
}

size_t LearningExplorerAgent::SelectAction(const WorldGrid &grid) {
  UpdateMemory(grid);

  const WorldPosition bfs_target = BFSNextStep(grid);

  const std::vector<size_t> candidate_actions = {
      GetActionID("up"), GetActionID("down"), GetActionID("left"),
      GetActionID("right")};

  double best_score = -std::numeric_limits<double>::infinity();
  size_t best_action = 0;

  for (size_t action_id : candidate_actions) {
    if (action_id == 0)
      continue;

    const double score = ScoreAction(grid, action_id, bfs_target);
    if (score > best_score) {
      best_score = score;
      best_action = action_id;
    }
  }

  prev_position = GetLocation().AsWorldPosition();
  has_prev_position = true;

  last_action = best_action;
  return best_action;
}

int LearningExplorerAgent::GetActionSuccessCount(size_t /* action_id */) const {
  return 0;
}

int LearningExplorerAgent::GetActionFailureCount(size_t /* action_id */) const {
  return 0;
}

int LearningExplorerAgent::GetVisitedCellCount(const WorldGrid &grid,
                                               WorldPosition pos) const {
  return GetVisitCount(grid, pos);
}

} // namespace cse498
