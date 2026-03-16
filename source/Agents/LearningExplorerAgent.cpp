#include "LearningExplorerAgent.hpp"

#include <limits>

namespace cse498 {

  LearningExplorerAgent::LearningExplorerAgent(size_t id,
                                               const std::string & name,
                                               const WorldBase & world)
    : AgentBase(id, name, world)
  { }

  bool LearningExplorerAgent::Initialize()
  {
    return HasAction("up") &&
           HasAction("down") &&
           HasAction("left") &&
           HasAction("right");
  }

  size_t LearningExplorerAgent::CellIndex(const WorldGrid & grid, WorldPosition pos) const
  {
    return pos.CellY() * grid.GetWidth() + pos.CellX();
  }

  int LearningExplorerAgent::GetVisitCount(const WorldGrid & grid, WorldPosition pos) const
  {
    const size_t index = CellIndex(grid, pos);
    auto it = visit_count_by_cell.find(index);
    if (it == visit_count_by_cell.end()) {
      return 0;
    }
    return it->second;
  }

  WorldPosition LearningExplorerAgent::PredictMove(WorldPosition pos, size_t action_id) const
  {
    if (action_id == GetActionID("up"))    return pos.Up();
    if (action_id == GetActionID("down"))  return pos.Down();
    if (action_id == GetActionID("left"))  return pos.Left();
    if (action_id == GetActionID("right")) return pos.Right();
    return pos;
  }

  void LearningExplorerAgent::UpdateMemory(const WorldGrid & grid)
  {
    // Mark the current location as visited.
    const WorldPosition current_pos = GetLocation().AsWorldPosition();
    visit_count_by_cell[CellIndex(grid, current_pos)]++;

    // Do not try to learn from a previous action on the very first turn.
    if (first_turn) {
      first_turn = false;
      return;
    }

    // Learn from whether the last action succeeded or failed.
    if (last_action != 0) {
      if (action_result != 0) {
        action_success_count[last_action]++;
      } else {
        action_failure_count[last_action]++;
      }
    }
  }

  double LearningExplorerAgent::ScoreAction(const WorldGrid & grid, size_t action_id) const
  {
    const WorldPosition current_pos = GetLocation().AsWorldPosition();
    const WorldPosition next_pos = PredictMove(current_pos, action_id);

    // Reject moves that go out of bounds.
    if (!grid.IsValid(next_pos)) {
      return -1'000'000.0;
    }

    // Reject moves into walls if this grid has a "wall" cell type.
    const size_t wall_id = grid.GetCellTypeID("wall");
    if (wall_id != 0 && grid[next_pos] == wall_id) {
      return -1'000'000.0;
    }

    double score = 0.0;

    // Strongly prefer unvisited cells.
    const int visits = GetVisitCount(grid, next_pos);
    if (visits == 0) {
      score += 10.0;
    } else {
      score -= static_cast<double>(visits);
    }

    // Reward actions that have worked in the past.
    auto success_it = action_success_count.find(action_id);
    if (success_it != action_success_count.end()) {
      score += static_cast<double>(success_it->second);
    }

    // Penalize actions that have failed in the past.
    auto failure_it = action_failure_count.find(action_id);
    if (failure_it != action_failure_count.end()) {
      score -= 3.0 * static_cast<double>(failure_it->second);
    }

    return score;
  }

  size_t LearningExplorerAgent::SelectAction(const WorldGrid & grid)
  {
    UpdateMemory(grid);

    const std::vector<size_t> candidate_actions = {
      GetActionID("up"),
      GetActionID("down"),
      GetActionID("left"),
      GetActionID("right")
    };

    double best_score = -std::numeric_limits<double>::infinity();
    size_t best_action = 0;

    for (size_t action_id : candidate_actions) {
      if (action_id == 0) continue;

      const double score = ScoreAction(grid, action_id);
      if (score > best_score) {
        best_score = score;
        best_action = action_id;
      }
    }

    last_action = best_action;
    return best_action;
  }

  int LearningExplorerAgent::GetActionSuccessCount(size_t action_id) const
  {
    auto it = action_success_count.find(action_id);
    return (it == action_success_count.end()) ? 0 : it->second;
  }

  int LearningExplorerAgent::GetActionFailureCount(size_t action_id) const
  {
    auto it = action_failure_count.find(action_id);
    return (it == action_failure_count.end()) ? 0 : it->second;
  }

  int LearningExplorerAgent::GetVisitedCellCount(const WorldGrid & grid, WorldPosition pos) const
  {
    return GetVisitCount(grid, pos);
  }

} // namespace cse498