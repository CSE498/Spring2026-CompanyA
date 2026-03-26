/**
 * @file Enemy.cpp
 * @author lrima
 */

#include "Enemy.hpp"

namespace cse498
{
    size_t Enemy::SelectAction(const WorldGrid& grid)
    {
        (void)grid;

        if (!mBehaviorRoot)
            return 0;

        BehaviorTrees::ExecutionContext ctx(mBlackboard);
        mBehaviorRoot->Tick(ctx);

        return mBlackboard.Get<size_t>("selected_action", 0);
    }

    std::size_t Enemy::ClaimGoldDrop()
    {
        if (mGoldClaimed)
        {
            return 0;
        }

        mGoldClaimed = true;
        return mGoldDrop;
    }
}
