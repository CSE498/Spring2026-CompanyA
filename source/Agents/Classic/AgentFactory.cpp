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
        const auto* player = world.TryGetAgent(0);
        if (player == nullptr || !player->IsAlive()) // if player missing/dead then not in range
            return Failure;

        if (IsInRange(enemy, player->GetPosition(), world.GetGrid()))
            return Success;
        return Failure;
    });
}


std::unique_ptr<Node> AgentFactory::AttackPlayer(const Enemy& enemy, const WorldBase& world) {
    return TreeBuilder::Act("Attack Player", [&world, &enemy](ExecutionContext&) {
        auto* player = world.TryGetAgent(0);
        if (player == nullptr || !player->IsAlive())
            return Failure;
        auto dmg = DamageCalculator::Calculate(enemy.GetStats(), player->GetStats());
        player->TakeDamage(dmg);
        return Success;
    });
}

std::unique_ptr<Node> AgentFactory::ChasePlayer(const Enemy& enemy, const WorldBase& world) {
    return TreeBuilder::Act("Chase Player", [&enemy, &world](ExecutionContext& ctx) {
        const auto* player = world.TryGetAgent(0);
        if (player == nullptr || !player->IsAlive()) // can't chase dead/missing players
            return Failure;

        const WorldPosition enemyPos = enemy.GetLocation().AsWorldPosition();
        const WorldPosition playerPos = player->GetPosition();

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
        if (action_name.empty()) {
            return Failure;
        }
        // Make sure the direction maps to a valid movement action string.

        // Sanity check: ensure the destination tile is walkable.
        if (!grid.IsWalkable(nextPos)) {
            return Failure;
        }

        size_t action_id = enemy.GetActionID(action_name);
        if (action_id == 0)
            return Failure;

        ctx.mBlackboard.Set<size_t>("selected_action", action_id);
        return Success;
    });
}


std::unique_ptr<Node> AgentFactory::RangeChasePlayer(const Enemy& enemy, const WorldBase& world) {
    //* If not within range of player chase + reset tile distance count
    return TreeBuilder::Act("(ranged) Chase Player", [&enemy, &world](ExecutionContext& ctx) {
        const auto* player = world.TryGetAgent(0);
        if (player == nullptr || !player->IsAlive()) // can't chase dead players
            return Failure;
        // We are either Far away or too close or In range but no line of sight
        // if we are far away
        if (PathGenerator::EuclideanDistance(enemy.GetPosition(), player->GetPosition()) > static_cast<double>(enemy.GetAtkRange())) {
            // then move closer
            ctx.mBlackboard.Set<size_t>("step_count", 0);
            auto path = PathGenerator::FindShortestPath(enemy.GetPosition(), player->GetPosition(),
                                                        PathRequest(world.GetGrid()));
            if (path && path.value().Size() >= 2) {
                ResolveMovement(enemy, path.value().At(1), ctx);
                return Success;
            }

            return Failure; // most likely trapped by walls. Can't do anything
        }
        // if we are too close (AKA < manhattan tiles away from player)
        if (PathGenerator::ManhattanDistance(enemy.GetPosition(), player->GetPosition()) < enemy.GetAtkRange()) {
            // Move further away
            if (ctx.mBlackboard.Get<size_t>("step_count", 0) >= SKELETON_MAX_STEP_AWAY_COUNT)
                return Failure; // No movement should be tried just attack.

            ctx.mBlackboard.Set<size_t>("step_count", ctx.mBlackboard.Get<size_t>("step_count", 0) + 1);

            auto path = PathGenerator::FindPointAway(enemy.GetPosition(), player->GetPosition(),
                                                     PathRequest(world.GetGrid()));
            // if empty then attack if possible
            if (path.Empty())
                return Failure;
            assert(path.Size() >= 2);
            ResolveMovement(enemy, path.At(1), ctx);
            return Success;
        }
        // otherwise last case: We are in range but no line of sight
        // just stay still. Player may leave then we can start chasing again or get closer and we run again
        // don't reset counter
        auto actionName = MovementTypes::GetActionName({0, 0});
        ctx.mBlackboard.Set<size_t>("selected_action", enemy.GetActionID(actionName));
        // No movement decided
        return Success;
    });
}


std::unique_ptr<Node> AgentFactory::IsPlayerInBoundedRange(const Enemy& enemy, const WorldBase& world) {
    return TreeBuilder::Act("(ranged) Chase Player", [&enemy, &world](ExecutionContext) {
        const auto* player = world.TryGetAgent(0);
        if (player == nullptr || !player->IsAlive()) {
            return Failure;
        }
        // If within range but still has Range tiles away from player then ... attack
        if (IsInRange(enemy, player->GetPosition(), world.GetGrid()) &&
            PathGenerator::ManhattanDistance(enemy.GetPosition(), player->GetPosition()) >= enemy.GetAtkRange()) {
            return Success;
        }

        return Failure;
    });
}




//////////////////////////////////////////////////////////////////////////////////////////
//
// Real Agent Implementations
//
//////////////////////////////////////////////////////////////////////////////////////////

std::unique_ptr<Node> AgentFactory::CreateTestFunctionTree(const Enemy& enemy, const WorldBase& world) {
    // Root node continually runs the enemy behavior.
    auto root = TreeBuilder::Repeat("Example Root");

    // Top-level selector: try to attack if in range; otherwise chase the player.
    auto selector = TreeBuilder::Sel("Example Behavior");

    // Attack sequence: only succeeds if the player is in range.
    auto attackSeq = TreeBuilder::Seq("Attack Player Seq");

    // NOTE: this blackboard execution context doesn't seem that nice to work with here ...
    // attackSeq->AddChild(TreeBuilder::Act("Player in Range", [enemy, &world](ExecutionContext& ctx)
    // Condition: is the player currently within this enemy's attack range?
    attackSeq->AddChild(IsPlayerInRange(enemy, world));
    attackSeq->AddChild(AttackPlayer(enemy, world));


    // Chase action: when not in range, move one step toward the player using PathGenerator.
    selector->AddChild(std::move(attackSeq));
    selector->AddChild(ChasePlayer(enemy, world));

    root->SetChild(std::move(selector));

    return root;
}


std::unique_ptr<Node> AgentFactory::CreateSkeletonTree(const Enemy& enemy, const WorldBase& world) {
    /*
     * The Tree:
     * If not within range of player chase + reset tile distance count
     * If within range but still has Range tiles away from player then attack
     * Otherwise assume player is chasing move further away and increment up to 5
     * If unable to move then attack
     */
    auto root = TreeBuilder::Repeat("Skeleton Root");
    auto selector = TreeBuilder::Sel("Skeleton Behavior");

    auto atkSeq = TreeBuilder::Seq("Skeleton Attack Sequence");
    // Check if player is in range and if so then attack
    atkSeq->AddChild(IsPlayerInBoundedRange(enemy, world));
    atkSeq->AddChild(AttackPlayer(enemy, world));
    // if ^^^ this returns false then 2 things:
    // 1. not in OPTIMAL range of player or 2. Player's dead. Ignore case 2 since it won't matter -->
    // not in range of player so move closer:
    auto moveSeq = TreeBuilder::Seq("Skeleton Move");
    // We need failure on movement and success on needing to continue to attack which is backwards.
    // I did it the intuitive way so invert comes in handy here
    auto temp = TreeBuilder::Inv("Inversion of range chase");
    temp->SetChild(RangeChasePlayer(enemy, world));
    moveSeq->AddChild(std::move(temp));
    // if failure then try to attack! because no movement could be done
    moveSeq->AddChild(IsPlayerInRange(enemy, world)); // UNBOUNDED -- We are close enough and can't move so attack
    moveSeq->AddChild(AttackPlayer(enemy, world));

    selector->AddChild(std::move(atkSeq));
    selector->AddChild(std::move(moveSeq));
    root->SetChild(std::move(selector));
    return root;
}

std::unique_ptr<Node> AgentFactory::CreateGoblinTree(const Enemy& enemy, const WorldBase& world) {
    auto root = TreeBuilder::Repeat("Goblin Root");

    auto selector = TreeBuilder::Sel("Goblin Behavior");

    auto attackSeq = TreeBuilder::Seq("Goblin Attack Seq");
    attackSeq->AddChild(IsPlayerInRange(enemy, world));
    attackSeq->AddChild(AttackPlayer(enemy, world));

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
    // Skeleton:
    /*
     * 1. Chases the player then
     * Once within range it will attack from afar.
     * If Chased it will run away for up to 5 tiles then after that will attack
     * until it is killed
     * If ever gets out of range of the player and has to move closer then the 5 tile count will reset
     */

    AgentStats stats = AgentLevels::GetSkeletonStats(def.mLevel);
    auto enemy = CreateAgent(def, stats, world); // createAgent can't return nullptr
    enemy->SetBehaviorTree(CreateSkeletonTree(*enemy, world));
    return enemy;
}

std::unique_ptr<Enemy> AgentFactory::CreateEnemyGoblin(const AgentDefinition& def, WorldBase& world) {
    AgentStats stats = AgentLevels::GetGoblinStats(def.mLevel);
    auto enemy = CreateAgent(def, stats, world); // createAgent can't return nullptr
    enemy->SetBuiltInWeaponName("Dagger");
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
    const WorldPosition& p1 = enemy.GetPosition();
    const WorldPosition& p2 = entityPosition;

    if (PathGenerator::IsPathClear(p1, p2 - p1, {{}, grid})) {
        // then we are pretty much good. Just check Euclidean Distance is less than range with respect to the player
        if ((p1 - p2).GetMagnitude() <= static_cast<double>(enemy.GetAtkRange()) + EP)
            return true;
    }
    return false;
}

void AgentFactory::ResolveMovement(const Enemy& enemy, const WorldPosition& newEnemyLocation, ExecutionContext& ctx) {
    PathVector neededDir = newEnemyLocation - enemy.GetPosition();
    auto actionName = MovementTypes::GetActionName(neededDir);
    ctx.mBlackboard.Set<size_t>("selected_action", enemy.GetActionID(actionName));
}

} // namespace cse498
