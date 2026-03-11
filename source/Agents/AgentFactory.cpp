/**
 * @file AgentFactory.cpp
 * @author lrima
 */

#include "AgentFactory.h"
#include "../tools/BehaviorTree/BehaviorTree.hpp"
#include "core/WorldBase.hpp"
#include "tools/AgentAbility.hpp"
#include "tools/PathGenerator.hpp"
#include "tools/PathVector.hpp"

using cse498::BehaviorTrees::TreeBuilder;
using cse498::BehaviorTrees::ExecutionContext;
using cse498::BehaviorTrees::Node;


namespace cse498
{
std::unique_ptr<Node> AgentFactory::CreateSkeletonTree(const Enemy* enemy, const WorldBase & world)
{
    auto root = TreeBuilder::Sel("Skeleton Root");

    // runs through in order until one fails (if player in range ... if this if that until attack)
    auto attackSeq = TreeBuilder::Seq("Attack Player Seq");

    // NOTE: this blackboard execution context doesn't seem that nice to work with here ...
    attackSeq->AddChild(TreeBuilder::Act("Player in Range", [enemy, &world](ExecutionContext& ctx)
    {
        if (enemy && IsInRange(*enemy, world.getPlayerPosition(), world.GetGrid()))
            return Node::Status::Success;
        return Node::Status::Failure;
    }));


    return root;
}

std::unique_ptr<Enemy> AgentFactory::CreateEnemySkeleton(const std::string & name, WorldBase & world)
{
    auto skeleton = std::make_unique<Enemy>(world.GetNextAgentId(), name, world);
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
        if ((p1 - p2).getMagnitude() < enemy.GetAttackRange())
            return true;
    }
    return false;


}

}
