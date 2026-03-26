#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>
#include <vector>

#include "../core/AgentBase.hpp"

namespace cse498 {

class LearningExplorerAgent : public AgentBase {
private:
  std::unordered_map<size_t, int> visit_count_by_cell;

  size_t last_action = 0;
  bool first_turn = true;

  WorldPosition prev_position{-1, -1};
  bool has_prev_position = false;

  [[nodiscard]] size_t CellIndex(const WorldGrid &grid,
                                 WorldPosition pos) const;

  [[nodiscard]] int GetVisitCount(const WorldGrid &grid,
                                  WorldPosition pos) const;

  [[nodiscard]] WorldPosition PredictMove(WorldPosition pos,
                                          size_t action_id) const;

  /// BFS to find the first step toward the nearest unvisited walkable cell.
  /// Returns current position if every reachable cell has been visited.
  [[nodiscard]] WorldPosition BFSNextStep(const WorldGrid &grid) const;

  [[nodiscard]] double ScoreAction(const WorldGrid &grid, size_t action_id,
                                   const WorldPosition &bfs_target) const;

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
