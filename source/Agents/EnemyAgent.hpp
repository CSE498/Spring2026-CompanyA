//
// Group1 Enemy Agent Class
//
#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include "../core/AgentBase.hpp"

namespace cse498
{
    class EnemyAgent : public AgentBase
    {
    private:
        // Predict where this move would place the enemy.
        [[nodiscard]] WorldPosition PredictMove(WorldPosition pos, size_t action_id) const;

        // Distance heuristic to the player.
        [[nodiscard]] int ManhattanDistance(WorldPosition a, WorldPosition b) const;

        // Score a candidate action.
        [[nodiscard]] double ScoreAction(const WorldGrid &grid, size_t action_id) const;

    public:
        EnemyAgent(size_t id, const std::string &name, const WorldBase &world);
        /**
         * Default Decontructor
         */
        ~EnemyAgent() override = default;
        bool Initialize() override;
        [[nodiscard]] size_t SelectAction(const WorldGrid &grid) override;
};

} // namespace cse498