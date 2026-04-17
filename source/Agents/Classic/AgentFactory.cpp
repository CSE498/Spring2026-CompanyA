/**
 * @file AgentFactory.cpp
 * @author lrima
 */

#include "AgentFactory.hpp"
#include "../../Worlds/DemoG2/WorldActions.hpp"
#include "../../core/AgentBase.hpp"
#include "../../core/WorldBase.hpp"
#include "../../tools/BehaviorTree.hpp"
#include "../../tools/DamageCalculator.hpp"
#include "../../tools/PathGenerator.hpp"
#include "../../tools/PathVector.hpp"

#include <cmath>

#include "MovementTypes.hpp"

using cse498::BehaviorTrees::ExecutionContext;
using cse498::BehaviorTrees::Node;
using cse498::BehaviorTrees::TreeBuilder;
using Node::Status::Failure;
using Node::Status::Success;

namespace cse498 {

//////////////////////////////////////////////////////////////////////////////////////////
//
// Helper functions to build out agents easily!
//
//////////////////////////////////////////////////////////////////////////////////////////


std::unique_ptr<Node> AgentFactory::IsPlayerInRange(const Enemy& enemy, const WorldBase& world) {
    return TreeBuilder::Act("Player in Range", [&world, &enemy](ExecutionContext&) {
        assert(enemy.IsAlive());
        if (world.GetPlayer()->IsAlive() == false)
            return Failure;

        if (IsInRange(enemy, world.GetPlayerPosition(), world.GetGrid()))
            return Success;
        return Failure;
    });
}


std::unique_ptr<Node> AgentFactory::Attack(const Enemy& enemy, const WorldBase& world) {
    return TreeBuilder::Act("Attack Player", [&world, &enemy](ExecutionContext&) {
        if (world.GetPlayer()->IsAlive() == false)
            return Failure;
        auto dmg = DamageCalculator::Calculate(enemy.GetStats(), world.GetPlayer()->GetStats());
        world.GetPlayer()->TakeDamage(dmg);
        return Success;
    });
}

std::unique_ptr<Node> AgentFactory::ChasePlayer(const Enemy& enemy, const WorldBase& world) {
    return TreeBuilder::Act("Chase Player", [&enemy, &world](ExecutionContext& ctx) {
        const WorldPosition enemyPos = enemy.GetLocation().AsWorldPosition();
        const WorldPosition playerPos = world.GetPlayerPosition();

        // Build a simple pathfinding request for this enemy on the world's main grid.
        const WorldGrid& grid = world.GetGrid();
        PathRequest request({}, grid);

        auto pathOpt = PathGenerator::FindShortestPath(enemyPos, playerPos, request);
        if (!pathOpt) {
            // No path found; fail so selector can try another behavior.
            return Failure;
        }

        const WorldPath& path = pathOpt.value();
        if (path.Size() == 1) // we are on the tile
        {
            // Path either only contains our current position or we're already at the player.
            return Success;
        }

        // The second point in the path is the next tile to step toward.
        WorldPosition nextPos = Round(path.At(1));
        WorldPosition curPos = Round(enemyPos);
        auto dir = nextPos - curPos;

        auto action_name = MovementTypes::GetActionName(dir);
        // Just checks that the next position is indeed in the 8 diagonal positions
        // otherwise action_name will be false from std::optional

        // Sanity check: ensure the destination tile is walkable.
        assert(grid.IsWalkable(nextPos));

        size_t action_id = enemy.GetActionID(action_name);
        if (action_id == 0)
            return Failure;

        ctx.mBlackboard.Set<size_t>("selected_action", action_id);
        return Success;
    });
}


//////////////////////////////////////////////////////////////////////////////////////////
//
// Real Agent Implementations
//
//////////////////////////////////////////////////////////////////////////////////////////


std::unique_ptr<Node> AgentFactory::CreateSkeletonTree(const Enemy& enemy, const WorldBase& world) {
    // Root node continually runs the enemy behavior.
    auto root = TreeBuilder::Repeat("Skeleton Root");

    // Top-level selector: try to attack if in range; otherwise chase the player.
    auto selector = TreeBuilder::Sel("Skeleton Behavior");

    // Attack sequence: only succeeds if the player is in range.
    auto attackSeq = TreeBuilder::Seq("Attack Player Seq");

    // NOTE: this blackboard execution context doesn't seem that nice to work with here ...
    // attackSeq->AddChild(TreeBuilder::Act("Player in Range", [enemy, &world](ExecutionContext& ctx)
    // Condition: is the player currently within this enemy's attack range?
    attackSeq->AddChild(IsPlayerInRange(enemy, world));
    attackSeq->AddChild(Attack(enemy, world));


    // Chase action: when not in range, move one step toward the player using PathGenerator.
    selector->AddChild(std::move(attackSeq));
    selector->AddChild(ChasePlayer(enemy, world));

    root->SetChild(std::move(selector));

    return root;
}

std::unique_ptr<Node> AgentFactory::CreateGoblinTree(const Enemy& enemy, const WorldBase& world) {
    auto root = TreeBuilder::Repeat("Goblin Root");

    auto selector = TreeBuilder::Sel("Goblin Behavior");

    auto attackSeq = TreeBuilder::Seq("Goblin Attack Seq");
    attackSeq->AddChild(IsPlayerInRange(enemy, world));
    attackSeq->AddChild(Attack(enemy, world));

    selector->AddChild(std::move(attackSeq));
    selector->AddChild(ChasePlayer(enemy, world));

    root->SetChild(std::move(selector));

    return root;
}

std::unique_ptr<Node> AgentFactory::CreatePatrolTree(AgentBase* agent) {
    return TreeBuilder::Act("Walk Back And Forth", [agent](ExecutionContext& ctx) {
        if (!agent)
            return Failure;
        auto dir = ctx.mBlackboard.Get<std::string>("patrol_direction", WorldActions::MOVE_LEFT_STRING);
        size_t actionId;
        if (dir == WorldActions::MOVE_LEFT_STRING) {
            actionId = agent->GetActionID(WorldActions::MOVE_LEFT_STRING);
            ctx.mBlackboard.Set<std::string>("patrol_direction", WorldActions::MOVE_RIGHT_STRING);
        } else {
            actionId = agent->GetActionID(WorldActions::MOVE_RIGHT_STRING);
            ctx.mBlackboard.Set<std::string>("patrol_direction", WorldActions::MOVE_LEFT_STRING);
        }
        if (actionId != 0)
            ctx.mBlackboard.Set<size_t>("selected_action", actionId);
        return Node::Status::Running;
    });
}

std::unique_ptr<Enemy> AgentFactory::CreatePatrolAgent(WorldBase& world, const WorldPosition& spawn) {
    auto patrol = std::make_unique<Enemy>(world.GetNextAgentId(), "Patrol", world);
    patrol->SetLocation(spawn);
    patrol->SetBehaviorTree(CreatePatrolTree(patrol.get()));
    return patrol;
}


std::unique_ptr<Node> AgentFactory::CreateEnemyFollowPlayerTree(Enemy* enemy, const WorldBase& world,
                                                                std::size_t targetAgentIndex) {
    /*
     * This is a demo tree because the demo works slightly different than practice due to not having all the
     * requests complete from the world groups. It is a compact version of the more detailed trees
     */
    auto chase = TreeBuilder::Act("ChaseOneStepTowardTarget", [enemy, &world, targetAgentIndex](ExecutionContext& ctx) {
        if (enemy == nullptr || !enemy->IsAlive()) {
            ctx.mBlackboard.Set<std::size_t>("selected_action", WorldActions::REMAIN_STILL);
            return Failure;
        }
        const auto target = world.TryGetAgent(targetAgentIndex);
        if (target == nullptr)
            return Failure;
        const WorldPosition enemyPos = enemy->GetLocation().AsWorldPosition();
        const WorldPosition targetPos = target->GetLocation().AsWorldPosition();


        if (IsAdjacentForCombat(enemyPos, targetPos)) {
            ctx.mBlackboard.Set<std::size_t>("selected_action", WorldActions::INTERACT);
            return Success;
        }
        const WorldPosition next = PathGenerator::NextCardinalToward(enemyPos, targetPos);
        const double dx = next.X() - enemyPos.X();
        const double dy = next.Y() - enemyPos.Y();
        const std::size_t aid = MovementTypes::GetActionID(dx, dy);
        ctx.mBlackboard.Set<std::size_t>("selected_action", aid);
        return Success;
    });
    return chase;
}


std::unique_ptr<Enemy> AgentFactory::CreateEnemySkeleton(const AgentDefinition& def, WorldBase& world) {
    AgentStats stats = AgentLevels::GetSkeletonStats(def.mLevel);
    auto enemy = CreateAgent(def, stats, world); // createAgent can't return nullptr
    enemy->SetBehaviorTree(CreateSkeletonTree(*enemy, world));
    return enemy;
}

std::unique_ptr<Enemy> AgentFactory::CreateEnemyGoblin(const AgentDefinition& def, WorldBase& world) {
    AgentStats stats = AgentLevels::GetGoblinStats(def.mLevel);
    auto enemy = CreateAgent(def, stats, world); // createAgent can't return nullptr
    enemy->SetBehaviorTree(CreateGoblinTree(*enemy, world));
    return enemy;
}

std::unique_ptr<Enemy> AgentFactory::CreateAgent(const AgentDefinition& def, const AgentStats& stats,
                                                 WorldBase& world) {
    auto agent = std::make_unique<Enemy>(world.GetNextAgentId(), def.mName, world);
    agent->SetLocation(def.mSpawn);
    agent->SetStats(stats);
    // atk can be used by combat later; no member on Enemy yet for minimal impl
    return agent;
}

/* ///////////////////////////////////////////////////////////////////////
 *
 * Helper functions
 *
 *////////////////////////////////////////////////////////////////////////

bool AgentFactory::IsInRange(const Enemy& enemy, const WorldPosition& entityPosition, const WorldGrid& grid) {
    const WorldPosition& p1 = enemy.GetLocation().AsWorldPosition();
    const WorldPosition& p2 = entityPosition;

    if (PathGenerator::IsPathClear(p1, p2 - p1, {{}, grid})) {
        // then we are pretty much good. Just check Euclidean Distance is less than range with respect to the player
        // hitbox. TODO: update once hitbox information is more well-defined
        if ((p1 - p2).GetMagnitude() < static_cast<double>(enemy.GetAtkRange()))
            return true;
    }
    return false;
}

} // namespace cse498
