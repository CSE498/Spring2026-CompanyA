/**
 * @file AgentFactory.hpp
 * @author lrima
 *
 * This generates agents instead of having individual classes for each variation
 * It sets certain properties and returns the created object
 */

#pragma once

#include <memory>
#include <optional>
#include <string>

#include "../../core/WorldPosition.hpp"
#include "../../tools/BehaviorTree.hpp"
#include "../../tools/PathVector.hpp"
#include "AgentDefinition.hpp"
#include "AgentLevels.hpp"
#include "Enemy.hpp"

namespace cse498 {

class WorldBase;

/// Minimal data to create an agent (e.g. skeleton) without a separate class per type.


class AgentFactory {
private:
    static constexpr int SKELETON_MAX_STEP_AWAY_COUNT = 5;
    ////////////////////////////////////////////////////////////////////////////////////
    //                  HELPER FUNCTIONS TO BUILD TREES
    ////////////////////////////////////////////////////////////////////////////////////
    /**
     * Creates an Action node for determining if the player is in range
     * @param enemy - enemy creature that owns this
     * @param world - world where player lives
     * @return Action node. True if player is in range
     */
    static std::unique_ptr<BehaviorTrees::Node> IsPlayerInRange(const Enemy& enemy, const WorldBase& world);
    static std::unique_ptr<BehaviorTrees::Node> AttackPlayer(const Enemy& enemy, const WorldBase& world);
    static std::unique_ptr<BehaviorTrees::Node> ChasePlayer(const Enemy& enemy, const WorldBase& world);
    static std::unique_ptr<BehaviorTrees::Node> RangeChasePlayer(const Enemy& enemy, const WorldBase& world);
    static std::unique_ptr<BehaviorTrees::Node> IsPlayerInBoundedRange(const Enemy& enemy, const WorldBase& world);
    /**
     * Creates the tree for the skeleton
     * @param enemy the skeleton that will own this tree
     * @param world - the world from the enemy
     * @return root node for the tree
     */
    static std::unique_ptr<BehaviorTrees::Node> CreateSkeletonTree(const Enemy& enemy, const WorldBase& world);

    /**
     * Creates the behavior tree for the goblin (attack in range, else chase).
     * @param enemy the goblin that will own this tree
     * @param world - the world from the enemy
     * @return root node for the tree
     */
    static std::unique_ptr<BehaviorTrees::Node> CreateGoblinTree(const Enemy& enemy, const WorldBase& world);


    /// Tree that alternates left/right every tick (for patrol agent).
    static std::unique_ptr<BehaviorTrees::Node> CreatePatrolTree(AgentBase* agent);

    /// Internal creation: build base enemy from definition at spawn; createX functions call this.
    static std::unique_ptr<Enemy> CreateAgent(const AgentDefinition& def, const AgentStats& stats, WorldBase& world);

    /**
     * Checks if the enemy who owns a property called range is in range of the entity -- pretty much anything with a
     * position. The check operates by looking at all tiles along a directly straight path to the entity
     * @param enemy - main character to check for
     * @param entityPosition - position to check for
     * @param grid - world grid for walkability/path checks
     * @return true if in range
     */
    static bool IsInRange(const Enemy& enemy, const WorldPosition& entityPosition, const WorldGrid& grid);

    /**
     * Helper function to follow the provided direction from A*-like searches
     * @param enemy - the enemy to move
     * @param newEnemyLocation - the new location resolved to move to for the enemy
     * @param ctx - context to update
     */
    static void ResolveMovement(const Enemy& enemy, const WorldPosition& newEnemyLocation,
                                BehaviorTrees::ExecutionContext& ctx);

    /**
     * Demo function - Checks two positions are adjacent
     * @param a
     * @param b
     * @return
     */
    [[nodiscard]] static bool IsAdjacentForCombat(const WorldPosition& a, const WorldPosition& b) {
        const double dx = std::abs(a.X() - b.X());
        const double dy = std::abs(a.Y() - b.Y());
        return dx <= 1.0 && dy <= 1.0 && (dx > 0.0 || dy > 0.0);
    }


public:
    /**
     * Create a skeleton enemy from a definition and spawn position.
     * @param def - agent definition
     * @param world - world
     * @return enemy object
     */
    static std::unique_ptr<Enemy> CreateEnemySkeleton(const AgentDefinition& def, WorldBase& world);
    /**
     * This should not be used by a world. This is for testing a series of private functions in simple example
     * structures to make sure implementations are working before adding even more complexity
     * @param enemy - the enemy who will own the tree
     * @param world - the world the enemy lives in
     * @return the root node
     */
    static std::unique_ptr<BehaviorTrees::Node> CreateTestFunctionTree(const Enemy& enemy, const WorldBase& world);

    /**
     * Create a goblin enemy from a definition and spawn position.
     * @param def - agent definition
     * @param world - world
     * @return enemy object
     */
    static std::unique_ptr<Enemy> CreateEnemyGoblin(const AgentDefinition& def, WorldBase& world);

    /**
     * Create an agent that walks left then right, repeatedly.
     * @param world - world
     * @param spawn - spawn loc
     * @return the enemy
     */
    static std::unique_ptr<Enemy> CreatePatrolAgent(WorldBase& world, const WorldPosition& spawn);

    static std::unique_ptr<BehaviorTrees::Node> CreateEnemyFollowPlayerTree(Enemy* enemy, const WorldBase& world,
                                                                            std::size_t targetAgentIndex);

    static std::unique_ptr<Enemy> CreateTestFunctionAgent(const AgentDefinition& def, WorldBase& world) {
        AgentStats stats = AgentLevels::GetSkeletonStats(def.mLevel);
        auto enemy = CreateAgent(def, stats, world); // createAgent can't return nullptr
        enemy->SetBehaviorTree(CreateTestFunctionTree(*enemy, world));
        return enemy;
    }
};


} // namespace cse498
