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

    [[nodiscard]] bool IsWalkable(const WorldGrid& grid, WorldPosition pos) const;

    [[nodiscard]] std::optional<WorldPosition> GetTargetPlayerPosition() const;
    [[nodiscard]] std::optional<size_t> AttackActionForAdjacentPlayer() const;

    [[nodiscard]] bool HasLineOfSight(const WorldGrid& grid, WorldPosition from, WorldPosition to) const;

    [[nodiscard]] std::optional<size_t> LineOfSightPursuitMove(const WorldGrid& grid, WorldPosition target) const;

    [[nodiscard]] std::optional<size_t> NextMoveToward(const WorldGrid& grid, WorldPosition target) const;

    [[nodiscard]] std::optional<size_t> ShortestAttackLaneMove(const WorldGrid& grid, WorldPosition target) const;

    [[nodiscard]] std::optional<size_t> ExploreMove(const WorldGrid& grid) const;

public:
    SmartEnemyAgent(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {}

        bool Initialize() override;
        [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;

        /**
         * @brief Identifies this agent as hostile so peer AI agents and the world
         *        treat it as an enemy (and so it will not pick itself as a target).
         * @see AgentBase::IsEnemy()
         * @return Always @c true.
         */
        [[nodiscard]] bool IsEnemy() const override { return true; }
    };

} // namespace cse498
