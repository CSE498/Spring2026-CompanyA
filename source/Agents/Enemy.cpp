/**
 * @file Enemy.cpp
 * @author lrima
 */

#include "Enemy.h"


namespace cse498
{


size_t Enemy::SelectAction(const WorldGrid &grid)
{
    (void)grid; // basically will be calling into behavior tree to tick or something?
    return 0;
}


}
