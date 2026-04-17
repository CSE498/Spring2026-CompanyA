/**
 * @file LearningExplorerAgent.hpp
 * @brief Explorer agent: prefers less-visited cells and uses BFS toward the
 *        nearest unvisited walkable cell.
 *
 * @details Does not target other agents or use combat/loot. Initialize()
 * requires up, down, left, right. See docs/Group1.md for intended use and
 *          comparison with other agents.
 */
#pragma once

#include <cstddef>
#include <string>
#include <unordered_map>

#include "../../core/AgentBase.hpp"

namespace cse498 {
/**
 * @class LearningExplorerAgent
 * @brief Agent that explores the grid by prioritizing unvisited cells.
 * This agent uses visit counts and BFS to guide movement toward the nearest unvisited cell.
 * Actions are scored to favor exploration, avoid obstacles and other agents, and reduce oscillation between positions.
 *
 * @note Requires movement actions: up, down, left, right.
 *
 * @inherits AgentBase
 */
class LearningExplorerAgent : public AgentBase {
private:
    /// Stores visit counts for each grid cell (indexed as a 1D value).
    std::unordered_map<size_t, int> mVisitCount;

    /// ID of the last action taken by the agent.
    size_t mLastAction = 0;
    /// Indicates whether the agent is on its first turn.
    bool mFirstTurn = true;

    /// Stores the agent's previous position.
    WorldPosition mPrevPosition{-1, -1};
    /// Indicates whether a valid previous position exists.
    bool mHasPrevPosition = false;

    [[nodiscard]] size_t CellIndex(const WorldGrid& grid, WorldPosition pos) const;

    [[nodiscard]] int GetVisitCount(const WorldGrid& grid, WorldPosition pos) const;

    [[nodiscard]] WorldPosition PredictMove(WorldPosition pos, size_t action_id) const;

    /// BFS to find the first step toward the nearest unvisited walkable cell.
    /// Returns current position if every reachable cell has been visited.
    [[nodiscard]] WorldPosition BFSNextStep(const WorldGrid& grid) const;

    [[nodiscard]] double ScoreAction(const WorldGrid& grid, size_t action_id, const WorldPosition& bfs_target) const;

    void UpdateMemory(const WorldGrid& grid);

public:
    /// Score assigned to invalid or undesirable actions.
    static constexpr double BadScore = -1'000'000.0;
    /// Bonus for moving to an unvisited cell.
    static constexpr double UnvisitedBonus = 100.0;
    /// Penalty applied per prior visit to a cell.
    static constexpr double RevisitPenaltyPerVisit = 5.0;
    /// Penalty for being close to another agent.
    static constexpr double AgentProximityPenalty = 50.0;
    /// Bonus for following BFS guidance toward unexplored areas.
    static constexpr double BfsGuidanceBonus = 200.0;
    /// Penalty for moving back to the previous position.
    static constexpr double OscillationPenalty = 30.0;
    LearningExplorerAgent(size_t id, const std::string& name, const WorldBase& world);
    /**
     * Default deconstructor
     */
    ~LearningExplorerAgent() override = default;
    bool Initialize() override;
    [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override;
    [[nodiscard]] int GetActionSuccessCount(size_t action_id) const;
    [[nodiscard]] int GetActionFailureCount(size_t action_id) const;
    [[nodiscard]] int GetVisitedCellCount(const WorldGrid& grid, WorldPosition pos) const;

    /**
     * @brief Getter function for last action taken
     *
     * @return the last action taken by the agent
     */
    [[nodiscard]] size_t GetLastAction() const { return mLastAction; }
};

} // namespace cse498
