/**
 * @file EnemyAgent.hpp
 * @brief Greedy grid “chaser” agent: picks a cardinal move that best reduces
 *        Manhattan distance to another agent (the first other agent from
 *        GetKnownAgents).
 *
 * @details Intended for maze-like worlds with walkable cells and walls.
 *          Requires movement actions: up, down, left, right. See docs/Group1.md
 *          for comparison with LearningExplorerAgent and TrailblazerAgent.
 */
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../core/AgentBase.hpp"

namespace cse498 {
    /**
     * @class EnemyAgent
     * @brief An AI-controlled agent that selects actions based on heuristic evaluation.
     *
     * The EnemyAgent evaluates possible actions using a scoring function that considers
     * predicted movement outcomes and distance to the player. It uses a Manhattan distance
     * heuristic and simulates potential moves to determine the most favorable action.
     *
     * Actions that result in undesirable states are assigned a low score (see BadScore).
     *
     * @inherits AgentBase
     */
    class EnemyAgent : public AgentBase {
    private:
        // Predict where this move would place the enemy.
        [[nodiscard]] WorldPosition PredictMove(WorldPosition pos, size_t action_id) const;

        // Distance heuristic to the player.
        [[nodiscard]] int ManhattanDistance(WorldPosition a, WorldPosition b) const;

        // Score a candidate action.
        [[nodiscard]] double ScoreAction(const WorldGrid &grid, size_t action_id) const;

    public:
        /// Bad score to prevent certain agent behavior
        static constexpr int BadScore = -1000000;

        EnemyAgent(size_t id, const std::string &name, const WorldBase &world);
        /**
         * Default Deconstructor
         */
        ~EnemyAgent() override = default;
        bool Initialize() override;
        [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;
    };

} // namespace cse498
