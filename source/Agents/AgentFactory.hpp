/**
 * @file AgentFactory.hpp
 * @author lrima
 *
 * This generates agents instead of having individual classes for each variation
 * It sets certain properties and returns the created object
 */

#ifndef CSE498COMPA_AGENTFACTORY_H
#define CSE498COMPA_AGENTFACTORY_H
#include <memory>
#include <string>

#include "Enemy.hpp"
#include "../core/WorldPosition.hpp"
#include "../tools/BehaviorTree/BehaviorTree.hpp"


namespace cse498 {

class WorldBase;

/// Minimal data to create an agent (e.g. skeleton) without a separate class per type.
struct AgentDefinition {
    std::string name = "agent";
    double hp = 100.0;
    double atk = 10.0;
};

class AgentFactory
{
private:
    static std::unique_ptr<BehaviorTrees::Node> CreateSkeletonTree(const Enemy* enemy, const WorldBase & world);

    /// Tree that alternates left/right every tick (for patrol agent).
    static std::unique_ptr<BehaviorTrees::Node> CreatePatrolTree(AgentBase* agent);

    /// Internal creation: build base enemy from definition at spawn; createX functions call this.
    static std::unique_ptr<Enemy> CreateAgent(const AgentDefinition& def, WorldBase& world, const WorldPosition& spawn);

    /**
     * Checks if the enemy who owns a property called range is in range of the entity -- pretty much anything with a
     * position. The check operates by looking at all tiles along a directly straight path to the entity
     * @param enemy - main character to check for
     * @param entityPosition - position to check for
     * @param grid - world grid for walkability/path checks
     * @return true if in range
     */
    static bool IsInRange(const Enemy &enemy, const WorldPosition &entityPosition, const WorldGrid & grid);
    [[nodiscard]] static  bool IsAdjacentForCombat(const WorldPosition& a, const WorldPosition& b) {
        const double dx = std::abs(a.X() - b.X());
        const double dy = std::abs(a.Y() - b.Y());
        return dx <= 1.0 && dy <= 1.0 && (dx > 0.0 || dy > 0.0);
    }



public:
    static std::unique_ptr<Enemy> CreateEnemySkeleton(const std::string & name, WorldBase & world);
    /// Create a skeleton enemy from a definition and spawn position.
    static std::unique_ptr<Enemy> CreateEnemySkeleton(const AgentDefinition& def, WorldBase & world, const WorldPosition& spawn);

    /// Create an agent that walks left then right, repeatedly. Uses action names "left" and "right".
    static std::unique_ptr<Enemy> CreatePatrolAgent(WorldBase& world, const WorldPosition& spawn);

    static std::unique_ptr<BehaviorTrees::Node> CreateEnemyFollowPlayerTree(Enemy *enemy,
                                                            const WorldBase &world,
                                                            std::size_t targetAgentIndex);
};


}


#endif //CSE498COMPA_AGENTFACTORY_H
