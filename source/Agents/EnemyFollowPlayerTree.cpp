#include "EnemyFollowPlayerTree.hpp"

#include <cmath>

#include "Enemy.h"
#include "../core/WorldBase.hpp"
#include "../tools/BehaviorTree/BehaviorTree.hpp"
#include "../Worlds/DemoSimpleWorldG2Actions.hpp"

namespace cse498 {

namespace {

namespace BT = ::cse498::BehaviorTrees;
namespace DemoAct = ::cse498::DemoSimpleWorldG2Actions;

/// One cardinal step toward `target` (matches DemoSimpleWorldG2 move actions, not diagonal).
[[nodiscard]] WorldPosition NextCardinalToward(WorldPosition from, WorldPosition target) {
    const double dx = target.X() - from.X();
    const double dy = target.Y() - from.Y();
    if (dx == 0.0 && dy == 0.0) {
        return from;
    }
    if (std::abs(dx) >= std::abs(dy)) {
        const double step_x = (dx == 0.0) ? 0.0 : (dx / std::abs(dx));
        return from.GetOffset(step_x, 0.0);
    }
    const double step_y = (dy == 0.0) ? 0.0 : (dy / std::abs(dy));
    return from.GetOffset(0.0, step_y);
}

/// Same adjacency rule as DemoSimpleWorldG2::HandleInteraction (Chebyshev ≤ 1, different cells).
[[nodiscard]] bool IsAdjacentForCombat(WorldPosition a, WorldPosition b) {
    const double dx = std::abs(a.X() - b.X());
    const double dy = std::abs(a.Y() - b.Y());
    return dx <= 1.0 && dy <= 1.0 && (dx > 0.0 || dy > 0.0);
}

[[nodiscard]] std::size_t DeltaToMoveAction(double dx, double dy) {
    if (dx == 0.0 && dy == -1.0) {
        return DemoAct::MOVE_UP;
    }
    if (dx == 0.0 && dy == 1.0) {
        return DemoAct::MOVE_DOWN;
    }
    if (dx == -1.0 && dy == 0.0) {
        return DemoAct::MOVE_LEFT;
    }
    if (dx == 1.0 && dy == 0.0) {
        return DemoAct::MOVE_RIGHT;
    }
    return DemoAct::REMAIN_STILL;
}

} // namespace

std::unique_ptr<BehaviorTrees::Node> CreateEnemyFollowPlayerTree(Enemy *enemy,
                                                                const WorldBase &world,
                                                                std::size_t targetAgentIndex) {
    auto chase = BT::TreeBuilder::Act(
        "ChaseOneStepTowardTarget",
        [enemy, &world, targetAgentIndex](BT::ExecutionContext &ctx) {
            if (enemy == nullptr || !enemy->IsAlive()) {
                ctx.mBlackboard.Set<std::size_t>("selected_action", DemoAct::REMAIN_STILL);
                return BT::Node::Status::Failure;
            }
            const WorldPosition epos = enemy->GetLocation().AsWorldPosition();
            const WorldPosition ppos = world.GetAgent(targetAgentIndex).GetLocation().AsWorldPosition();
            if (IsAdjacentForCombat(epos, ppos)) {
                ctx.mBlackboard.Set<std::size_t>("selected_action", DemoAct::INTERACT);
                return BT::Node::Status::Success;
            }
            const WorldPosition next = NextCardinalToward(epos, ppos);
            const double dx = next.X() - epos.X();
            const double dy = next.Y() - epos.Y();
            const std::size_t aid = DeltaToMoveAction(dx, dy);
            ctx.mBlackboard.Set<std::size_t>("selected_action", aid);
            return BT::Node::Status::Success;
        });
    return chase;
}

} // namespace cse498
