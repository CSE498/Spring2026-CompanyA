#pragma once

#include <deque>
#include <optional>
#include <unordered_map>

#include "../../core/AgentBase.hpp"

namespace cse498 {

/**
 * @class SmartEnemyAgent
 * @brief An AI-controlled enemy agent that intelligently pursues and attacks other agents.
 * The SmartEnemyAgent extends AgentBase and implements a decision-making system.
 * This class is designed to produce more strategic and natural enemy behavior compared to simple random or greedy
 * movement patterns.
 *
 * @note Assumes other agents in the world represent valid targets (e.g., the player). (For now)
 *
 * @inherits AgentBase
 */
class SmartEnemyAgent : public AgentBase {
private:
    /// The Agent holds the number of times it has visited a tile
    std::unordered_map<WorldPosition, size_t> mVisitCounts;

    /// Holds a queue of recently visited positions
    std::deque<WorldPosition> mRecentPositions;

    /// Check if a position is walkable.
    [[nodiscard]] bool IsWalkable(const WorldGrid& grid, WorldPosition pos) const;

    /// Get the position of the target player.
    [[nodiscard]] std::optional<WorldPosition> GetTargetPlayerPosition() const;

    /// Get the attack action for an adjacent player.
    [[nodiscard]] std::optional<size_t> AttackActionForAdjacentPlayer() const;

    /// Check if there is a line of sight between two positions.
    [[nodiscard]] bool HasLineOfSight(const WorldGrid& grid, WorldPosition from, WorldPosition to) const;

    /// Get the line of sight pursuit move.
    [[nodiscard]] std::optional<size_t> LineOfSightPursuitMove(const WorldGrid& grid, WorldPosition target) const;

    /// Get the next move toward a target using breadth-first search.
    [[nodiscard]] std::optional<size_t> NextMoveToward(const WorldGrid& grid, WorldPosition target) const;

    /// Get the shortest attack lane move.
    [[nodiscard]] std::optional<size_t> ShortestAttackLaneMove(const WorldGrid& grid, WorldPosition target) const;

    /// Get the exploration move.
    [[nodiscard]] std::optional<size_t> ExploreMove(const WorldGrid& grid) const;

public:
    /// Constructor for the SmartEnemyAgent.
    SmartEnemyAgent(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {}

    /// Initialize the SmartEnemyAgent.
    bool Initialize() override;

    /// Select an action for the SmartEnemyAgent.
    [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override;
};

} // namespace cse498
