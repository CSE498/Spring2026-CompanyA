#pragma once

#include <deque>
#include <optional>
#include <unordered_map>

#include "../core/AgentBase.hpp"

namespace cse498 {

    class SmartEnemyAgent : public AgentBase {
    private:
        std::unordered_map<WorldPosition, size_t> mVisitCounts;
        std::deque<WorldPosition> mRecentPositions;

        [[nodiscard]] bool IsWalkable(const WorldGrid &grid, WorldPosition pos) const;

        [[nodiscard]] std::optional<WorldPosition> GetTargetPlayerPosition() const;
        [[nodiscard]] std::optional<size_t> AttackActionForAdjacentPlayer() const;

        [[nodiscard]] bool HasLineOfSight(const WorldGrid &grid, WorldPosition from, WorldPosition to) const;

        [[nodiscard]] std::optional<size_t> LineOfSightPursuitMove(const WorldGrid &grid, WorldPosition target) const;

        [[nodiscard]] std::optional<size_t> NextMoveToward(const WorldGrid &grid, WorldPosition target) const;

        [[nodiscard]] std::optional<size_t> ShortestAttackLaneMove(const WorldGrid &grid, WorldPosition target) const;

        [[nodiscard]] std::optional<size_t> ExploreMove(const WorldGrid &grid) const;

    public:
        SmartEnemyAgent(size_t id, const std::string &name, const WorldBase &world) : AgentBase(id, name, world) {}

        bool Initialize() override;
        [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;
    };

} // namespace cse498
