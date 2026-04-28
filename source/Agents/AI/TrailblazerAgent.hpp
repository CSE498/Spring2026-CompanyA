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
        /// Move toward the nearest healing item.
        MoveToHeal,
        /// Pick up a healing item on the current tile.
        PickUpHeal,
        /// Consume a healing item (if available).
        UseHeal,
        /// Move toward the nearest weapon item.
        MoveToWeapon,
        /// Pick up a weapon on the current tile.
        PickUpWeapon,
        /// Move toward the nearest enemy (for engagement).
        MoveToEnemy,
        /// Attack an enemy (typically when adjacent / in range).
        AttackEnemy,
        /// Explore the map when no higher-priority goal applies.
        Explore
    };

private:
    /// The Agent holds the number of times it has visited a tile
    std::unordered_map<WorldPosition, size_t> mVisitCounts;

    /// Holds a queue of recently visited positions
    std::deque<WorldPosition> mRecentPositions;

    /**
     * @brief Check whether a position is traversable for navigation.
     * @param grid The world grid.
     * @param pos Position to test.
     */
    [[nodiscard]] bool IsWalkable(const WorldGrid& grid, WorldPosition pos) const;

    /**
     * @brief Find the nearest item position of a requested type.
     * @param heal_item If true, search for healing items; otherwise search for weapons.
     * @return Position of the nearest matching item, or `std::nullopt` if none exist.
     */
    [[nodiscard]] std::optional<WorldPosition> NearestItemPosition(bool heal_item) const;

    /**
     * @brief Find the nearest enemy position (if any).
     * @return Position of the nearest enemy, or `std::nullopt` if none exist.
     */
    [[nodiscard]] std::optional<WorldPosition> NearestEnemyPosition() const;

    /**
     * @brief Compute the next move toward a target position.
     * @param grid The world grid.
     * @param target Target position to path toward.
     * @param allow_enemy_target Whether the target may be an enemy-occupied tile.
     * @return An action index representing the chosen move, or `std::nullopt` if unreachable.
     */
    [[nodiscard]] std::optional<size_t> NextMoveToward(const WorldGrid& grid, WorldPosition target,
                                                       bool allow_enemy_target) const;

    /**
     * @brief Choose a chasing move toward a specific enemy.
     * @param grid The world grid.
     * @param enemy_pos The enemy position to chase.
     * @return An action index for the next move, or `std::nullopt` if no chase move is available.
     */
    [[nodiscard]] std::optional<size_t> ChaseEnemyMove(const WorldGrid& grid, WorldPosition enemy_pos) const;

    /**
     * @brief Choose an exploration move, biased away from over-visited areas.
     * @param grid The world grid.
     * @return An action index for the exploration move, or `std::nullopt` if no move is available.
     */
    [[nodiscard]] std::optional<size_t> ExploreMove(const WorldGrid& grid) const;

    /**
     * @brief If an enemy is adjacent, compute the corresponding attack action.
     * @return An action index for attacking an adjacent enemy, or `std::nullopt` if none are adjacent.
     */
    [[nodiscard]] std::optional<size_t> AttackActionForAdjacentEnemy() const;

    /**
     * @brief Build a short plan (sequence of high-level actions) for the current state.
     * @param grid The world grid.
     * @return Planned action sequence (may be empty if no plan is applicable).
     */
    [[nodiscard]] std::vector<PlanAction> BuildPlan(const WorldGrid& grid) const;

    /**
     * @brief Convert a high-level plan action into a concrete world action.
     * @param grid The world grid.
     * @param action The plan action to execute.
     * @return Concrete action index to submit to the world, or `std::nullopt` if not executable.
     */
    [[nodiscard]] std::optional<size_t> ExecutePlanAction(const WorldGrid& grid, PlanAction action) const;

public:
    /**
     * @brief Construct the agent.
     * @param id Unique agent id.
     * @param name Human-readable agent name.
     * @param world World instance this agent belongs to.
     */
    TrailblazerAgent(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {}

    /**
     * @brief Initialize internal state before the simulation starts.
     * @return True if initialization succeeded.
     */
    bool Initialize() override;

    /**
     * @brief Select the next action for the current world state.
     * @param grid The world grid snapshot for this decision step.
     * @return Concrete action index to execute (as defined by the world/engine).
     */
    [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override;
};

} // namespace cse498
