/**
 * @file AgentFactory.h
 * @author lrima
 *
 * This generates agents instead of having individual classes for each variation
 * It sets certain properties and returns the created object
 */

#ifndef CSE498COMPA_AGENTFACTORY_H
#define CSE498COMPA_AGENTFACTORY_H
#include <memory>
#include <string>

#include "Enemy.h"
#include "../core/WorldPosition.hpp"
#include "../tools/BehaviorTree/BehaviorTree.hpp"

using cse498::BehaviorTrees::Node;

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
    static std::unique_ptr<Node> CreateSkeletonTree(const Enemy* enemy, const WorldBase & world);

    /// Tree that alternates left/right every tick (for patrol agent).
    static std::unique_ptr<Node> CreatePatrolTree(AgentBase* agent);

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

public:
    static std::unique_ptr<Enemy> CreateEnemySkeleton(const std::string & name, WorldBase & world);
    /// Create a skeleton enemy from a definition and spawn position.
    static std::unique_ptr<Enemy> CreateEnemySkeleton(const AgentDefinition& def, WorldBase & world, const WorldPosition& spawn);

    /// Create an agent that walks left then right, repeatedly. Uses action names "left" and "right".
    static std::unique_ptr<Enemy> CreatePatrolAgent(WorldBase& world, const WorldPosition& spawn);
};


}


#endif //CSE498COMPA_AGENTFACTORY_H
