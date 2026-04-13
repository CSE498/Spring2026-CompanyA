/**
 * @file TrailblazerAgent.hpp
 * @brief Goal-oriented agent: heal when hurt, collect weapon/heal
 *        loot, attack adjacent enemies, otherwise explore.
 *
 * @details Uses AIWorld APIs (items, enemies); run inside
 *          AIWorld for full behavior. See docs/Group1.md for requirements and
 *          comparison with EnemyAgent.
 */
#pragma once

#include <deque>
#include <optional>
#include <unordered_map>
#include <vector>

#include "../../core/AgentBase.hpp"

namespace cse498 {

    /**
     * @class TrailblazerAgent
     * @brief Goal-oriented agent that balances survival, combat, and exploration.
     *
     * This agent uses a GOAP (Goal-Oriented Action Planning) approach to decide
     * actions based on its current state. It prioritizes:
     * - Healing when low on HP
     * - Collecting weapons or healing items
     * - Attacking enemies when possible
     * - Exploring when no higher-priority goal exists
     *
     * The agent also uses BFS for navigation and visit tracking to guide exploration.
     *
     * @note Designed to run within AIWorld for full functionality.
     */
    class TrailblazerAgent : public AgentBase {
    public:
        /// High-level actions used in GOAP planning.
        enum class PlanAction {
            MoveToHeal,
            PickUpHeal,
            UseHeal,
            MoveToWeapon,
            PickUpWeapon,
            MoveToEnemy,
            AttackEnemy,
            Explore
        };

    private:
        /// The Agent holds the number of times it has visited a tile
        std::unordered_map<WorldPosition, size_t> mVisitCounts;

        /// Holds a queue of recently visited positions
        std::deque<WorldPosition> mRecentPositions;

        [[nodiscard]] bool IsWalkable(const WorldGrid &grid, WorldPosition pos) const;
        [[nodiscard]] std::optional<WorldPosition> NearestItemPosition(bool heal_item) const;
        [[nodiscard]] std::optional<WorldPosition> NearestEnemyPosition() const;
        [[nodiscard]] std::optional<size_t> NextMoveToward(const WorldGrid &grid, WorldPosition target,
                                                           bool allow_enemy_target) const;
        [[nodiscard]] std::optional<size_t> ChaseEnemyMove(const WorldGrid &grid, WorldPosition enemy_pos) const;
        [[nodiscard]] std::optional<size_t> ExploreMove(const WorldGrid &grid) const;
        [[nodiscard]] std::optional<size_t> AttackActionForAdjacentEnemy() const;
        [[nodiscard]] std::vector<PlanAction> BuildPlan(const WorldGrid &grid) const;
        [[nodiscard]] std::optional<size_t> ExecutePlanAction(const WorldGrid &grid, PlanAction action) const;

    public:
        TrailblazerAgent(size_t id, const std::string &name, const WorldBase &world) : AgentBase(id, name, world) {}

        bool Initialize() override;
        [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;
    };

} // namespace cse498
