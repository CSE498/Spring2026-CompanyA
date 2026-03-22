/**
 * @file AgentFactory.cpp
 * @author lrima
 */

#include "AgentFactory.h"
#include "../core/AgentBase.hpp"
#include "../tools/BehaviorTree/BehaviorTree.hpp"
#include "../core/WorldBase.hpp"
#include "../tools/AgentAbility.hpp"
#include "../tools/PathGenerator.hpp"
#include "../tools/PathVector.hpp"
#include <cmath>

using cse498::BehaviorTrees::TreeBuilder;
using cse498::BehaviorTrees::ExecutionContext;
using cse498::BehaviorTrees::Node;

namespace cse498
{

std::unique_ptr<Node> AgentFactory::CreateSkeletonTree(const Enemy* enemy, const WorldBase & world)
{
    // Root node continually runs the enemy behavior.
    auto root = TreeBuilder::Repeat("Skeleton Root");

    // Top-level selector: try to attack if in range; otherwise chase the player.
    auto selector = TreeBuilder::Sel("Skeleton Behavior");

    // Attack sequence: only succeeds if the player is in range.
    auto attackSeq = TreeBuilder::Seq("Attack Player Seq");

    // NOTE: this blackboard execution context doesn't seem that nice to work with here ...
    // attackSeq->AddChild(TreeBuilder::Act("Player in Range", [enemy, &world](ExecutionContext& ctx)
    // Condition: is the player currently within this enemy's attack range?
    attackSeq->AddChild(TreeBuilder::Act("Player in Range", [enemy, &world](ExecutionContext&)
    {
        if (enemy && IsInRange(*enemy, world.getPlayerPosition(), world.GetGrid()))
            return Node::Status::Success;
        return Node::Status::Failure;
    }));

    // Placeholder "attack" action: when in range, stop moving (no-op for now).
    attackSeq->AddChild(TreeBuilder::Act("Attack (No-op)", [enemy]([[maybe_unused]] ExecutionContext& ctx)
    {
        if (!enemy)
            return Node::Status::Failure;

        // No-op attack: enemy will stand still once in range.
        return Node::Status::Success;
    }));

    // Chase action: when not in range, move one step toward the player using PathGenerator.
    auto chasePlayer = TreeBuilder::Act("Chase Player", [enemy, &world](ExecutionContext& ctx)
    {
        if (!enemy)
            return Node::Status::Failure;

        const WorldPosition enemyPos = enemy->GetLocation().AsWorldPosition();
        const WorldPosition playerPos = world.getPlayerPosition();

        // Build a simple pathfinding request for this enemy on the world's main grid.
        const WorldGrid &grid = world.GetGrid();
        PathRequest request({}, AgentAbility(), grid);

        auto pathOpt = PathGenerator::FindShortestPath(enemyPos, playerPos, request);
        if (!pathOpt)
        {
            // No path found; fail so selector can try another behavior.
            return Node::Status::Failure;
        }

        const WorldPath &path = pathOpt.value();
        if (path.Size() < 2)
        {
            // Path either only contains our current position or we're already at the player.
            return Node::Status::Success;
        }

        // The second point in the path is the next tile to step toward.
        WorldPosition nextPos = Round(path.At(1));
        WorldPosition curPos  = Round(enemyPos);

        std::string action_name;

        if (nextPos.CellX() > curPos.CellX())
        {
            action_name = "right";
        }
        else if (nextPos.CellX() < curPos.CellX())
        {
            action_name = "left";
        }
        else if (nextPos.CellY() > curPos.CellY())
        {
            action_name = "down";
        }
        else if (nextPos.CellY() < curPos.CellY())
        {
            action_name = "up";
        }
        else
        {
            // No movement between tiles; nothing to do.
            return Node::Status::Success;
        }

        // Sanity check: ensure the destination tile is walkable.
        if (!grid.IsWalkable(nextPos))
            return Node::Status::Failure;

        size_t action_id = enemy->GetActionID(action_name);
        if (action_id == 0)
            return Node::Status::Failure;

        ctx.blackboard.Set<size_t>("selected_action", action_id);
        return Node::Status::Success;
    });

    selector->AddChild(std::move(attackSeq));
    selector->AddChild(std::move(chasePlayer));

    root->SetChild(std::move(selector));

    return root;
}

std::unique_ptr<Node> AgentFactory::CreatePatrolTree(AgentBase* agent)
{
    return TreeBuilder::Act("Walk Back And Forth", [agent](ExecutionContext& ctx) {
        if (!agent) return Node::Status::Failure;
        std::string dir = ctx.blackboard.Get<std::string>("patrol_direction", "left");
        size_t actionId;
        if (dir == "left") {
            actionId = agent->GetActionID("left");
            ctx.blackboard.Set<std::string>("patrol_direction", "right");
        } else {
            actionId = agent->GetActionID("right");
            ctx.blackboard.Set<std::string>("patrol_direction", "left");
        }
        if (actionId != 0)
            ctx.blackboard.Set<size_t>("selected_action", actionId);
        return Node::Status::Running;
    });
}

std::unique_ptr<Enemy> AgentFactory::CreatePatrolAgent(WorldBase& world, const WorldPosition& spawn)
{
    auto patrol = std::make_unique<Enemy>(world.GetNextAgentId(), "Patrol", world);
    patrol->SetLocation(spawn);
    patrol->AddAction("left", 1).AddAction("right", 2);
    patrol->SetBehaviorTree(CreatePatrolTree(patrol.get()));
    return patrol;
}

std::unique_ptr<Enemy> AgentFactory::CreateEnemySkeleton(const std::string & name, WorldBase & world)
{
    AgentDefinition def;
    def.name = name;
    WorldPosition spawn(0, 0); // caller can move after add
    return CreateAgent(def, world, spawn);
}

std::unique_ptr<Enemy> AgentFactory::CreateEnemySkeleton(const AgentDefinition& def, WorldBase & world, const WorldPosition& spawn)
{
    return CreateAgent(def, world, spawn);
}

std::unique_ptr<Enemy> AgentFactory::CreateAgent(const AgentDefinition& def, WorldBase& world, const WorldPosition& spawn)
{
    auto skeleton = std::make_unique<Enemy>(world.GetNextAgentId(), def.name, world);
    skeleton->SetLocation(spawn);
    skeleton->SetMaxHealth(def.hp);
    skeleton->SetHealth(def.hp);
    // atk can be used by combat later; no member on Enemy yet for minimal impl
    auto root = CreateSkeletonTree(skeleton.get(), world);
    skeleton->SetBehaviorTree(std::move(root));
    return skeleton;
}

/* ///////////////////////////////////////////////////////////////////////
 *
 * Helper functions
 *
 *////////////////////////////////////////////////////////////////////////

bool AgentFactory::IsInRange(const Enemy &enemy, const WorldPosition &entityPosition, const WorldGrid & grid)
{
    const WorldPosition& p1 = enemy.GetLocation().AsWorldPosition();
    const WorldPosition& p2 = entityPosition;

    if (PathGenerator::IsPathClear(p1, p2 - p1, {{}, AgentAbility(), grid}))
    {
        // then we are pretty much good. Just check Euclidean Distance is less than range with respect to the player
        // hitbox. TODO: update once hitbox information is more well-defined
        if ((p1 - p2).GetMagnitude() < enemy.GetAttackRange())
            return true;
    }
    return false;


}

}
