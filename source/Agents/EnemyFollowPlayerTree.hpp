/**
 * @brief Behavior tree: enemy takes one grid step toward a target agent (e.g. player) per tick.
 */

#pragma once

#include <cstddef>
#include <memory>

namespace cse498 {

class Enemy;
class WorldBase;

namespace BehaviorTrees {
class Node;
}

/// Builds a tree that sets blackboard `selected_action` to movement ids from DemoSimpleWorldG2Actions.
[[nodiscard]] std::unique_ptr<BehaviorTrees::Node> CreateEnemyFollowPlayerTree(
    Enemy *enemy,
    const WorldBase &world,
    std::size_t targetAgentIndex);

} // namespace cse498
