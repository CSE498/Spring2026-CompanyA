/**
 * @file MovementTypes.cpp
 * @author Logan Rimarcik, Jacob
 */

#include "MovementTypes.hpp"

#include "tools/PathVector.hpp"
#include "Worlds/WorldActions.hpp"


namespace cse498
{

std::size_t MovementTypes::GetActionID(double dx, double dy) {
    if (dx == 0.0 && dy == -1.0) {
        return WorldActions::MOVE_UP;
    }
    if (dx == 0.0 && dy == 1.0) {
        return WorldActions::MOVE_DOWN;
    }
    if (dx == -1.0 && dy == 0.0) {
        return WorldActions::MOVE_LEFT;
    }
    if (dx == 1.0 && dy == 0.0) {
        return WorldActions::MOVE_RIGHT;
    }
    return WorldActions::REMAIN_STILL;
}


size_t MovementTypes::GetActionID(const PathVector &path)
{
    return GetActionID(path.X(), path.Y());

}
std::string MovementTypes::GetActionName(double dx, double dy)
{
    if (dx == 0.0 && dy == -1.0) {
        return WorldActions::MOVE_UP_STRING;
    }
    if (dx == 0.0 && dy == 1.0) {
        return WorldActions::MOVE_DOWN_STRING;
    }
    if (dx == -1.0 && dy == 0.0) {
        return WorldActions::MOVE_LEFT_STRING;
    }
    if (dx == 1.0 && dy == 0.0) {
        return WorldActions::MOVE_RIGHT_STRING;
    }
    return WorldActions::REMAIN_STILL_STRING;

}
std::string MovementTypes::GetActionName(const PathVector &path)
{
    return GetActionName(path.X(), path.Y());
}
}
