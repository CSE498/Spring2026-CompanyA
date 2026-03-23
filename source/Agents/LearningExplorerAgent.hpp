#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/AgentBase.hpp"

namespace cse498 {

class LearningExplorerAgent : public AgentBase {
private:
  // Track how many times each action has succeeded or failed.
  std::unordered_map<size_t, int> action_success_count;
  std::unordered_map<size_t, int> action_failure_count;

  // Track how often each cell has been visited.
  std::unordered_map<size_t, int> visit_count_by_cell;

  // Remember the last action chosen so we can learn from its result next turn.
  size_t last_action = 0;

  // Used so we do not try to learn from a nonexistent "previous move" on
  // turn 1.
  bool first_turn = true;

  // Helper: convert a world position into a unique cell index.
  [[nodiscard]] size_t CellIndex(const WorldGrid &grid,
                                 WorldPosition pos) const;

  // Helper: how many times has this cell been visited?
  [[nodiscard]] int GetVisitCount(const WorldGrid &grid,
                                  WorldPosition pos) const;

  // Predict the result of taking a specific movement action.
  [[nodiscard]] WorldPosition PredictMove(WorldPosition pos,
                                          size_t action_id) const;

  // Score an action: higher is better.
  [[nodiscard]] double ScoreAction(const WorldGrid &grid,
                                   size_t action_id) const;

  // Update internal learning state based on the previous action result.
  void UpdateMemory(const WorldGrid &grid);

public:
  LearningExplorerAgent(size_t id, const std::string &name,
                        const WorldBase &world);
  ~LearningExplorerAgent() override = default;

  bool Initialize() override;
  [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;

  [[nodiscard]] int GetActionSuccessCount(size_t action_id) const;
  [[nodiscard]] int GetActionFailureCount(size_t action_id) const;
  [[nodiscard]] int GetVisitedCellCount(const WorldGrid &grid,
                                        WorldPosition pos) const;
  [[nodiscard]] size_t GetLastAction() const { return last_action; }
};

} // namespace cse498