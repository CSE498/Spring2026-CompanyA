/**
 * @file Enemy.cpp
 * @author lrima
 */

#include "Enemy.h"

namespace cse498
{

size_t Enemy::SelectAction(const WorldGrid &grid)
{
    (void)grid;

    if (!mBehaviorRoot)
        return 0;

    BehaviorTrees::Blackboard blackboard;
    BehaviorTrees::ExecutionContext ctx(blackboard);

    mBehaviorRoot->Tick(ctx);

    return blackboard.Get<size_t>("selected_action", 0);
}

}
