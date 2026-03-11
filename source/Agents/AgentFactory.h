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

#include "Enemy.h"
#include "tools/BehaviorTree/BehaviorTree.hpp"

using cse498::BehaviorTrees::Node;

namespace cse498 {

class AgentFactory
{
private:
    static std::unique_ptr<Node> CreateSkeletonTree(const Enemy* enemy, const WorldBase & world);

    /**
     * Checks if the enemy who owns a property called range is in range of the entity -- pretty much anything with a
     * position. The check operates by looking at all tiles along a directly straight path to the entity
     * @param enemy - main character to check for
     * @param entity - object to check for
     * @return
     */
    static bool IsInRange(const Enemy &enemy, const WorldPosition &entityPosition, const WorldGrid & grid);

public:
    static std::unique_ptr<Enemy> CreateEnemySkeleton(const std::string & name, WorldBase & world);

};


}



#endif //CSE498COMPA_AGENTFACTORY_H