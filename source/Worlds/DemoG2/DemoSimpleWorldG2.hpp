/**
 * @brief Group 2 demo world: text map, player + farmer + enemy.
 */

#pragma once

#include "../../core/WorldBase.hpp"


namespace cse498 {
    class MerchantAgent;
    class Enemy;

    class DemoSimpleWorldG2 : public WorldBase {
    private:
        size_t mFloorId = 0;
        size_t mWallId = 0;

        size_t mPlayerId;
        size_t mFarmerId;
        size_t mEnemyId;

        [[nodiscard]] bool IsOccupiedByAgent(WorldPosition pos, const AgentBase *skip) const;

        void PrintWorldState() const;

        bool MoveAgentBy(AgentBase &agent, double dx, double dy);

        int HandleInteraction(AgentBase &actor);

        int HandleMerchantTrade(MerchantAgent &merchant) const;

        static void HandleEnemyDefeat(Enemy &enemy, PlayerAgent &player);

    protected:
        void ConfigAgent(AgentBase &agent) override;

    public:
        DemoSimpleWorldG2();

        int DoAction(AgentBase &agent, size_t action_id) override;

        void Run() override;
    };
} // namespace cse498
