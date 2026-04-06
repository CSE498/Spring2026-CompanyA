/**
 * @file MovementTypes.cpp
 * @author Logan Rimarcik, Jacob
 */

#include "MovementTypes.hpp"

#include "tools/PathVector.hpp"
#include "Worlds/DemoSimpleWorldG2Actions.hpp"

namespace G2 = cse498::DemoSimpleWorldG2Actions;

namespace cse498
{

std::size_t MovementTypes::GetActionID(double dx, double dy) {
    if (dx == 0.0 && dy == -1.0) {
        return G2::MOVE_UP;
    }
    if (dx == 0.0 && dy == 1.0) {
        return G2::MOVE_DOWN;
    }
    if (dx == -1.0 && dy == 0.0) {
        return G2::MOVE_LEFT;
    }
    if (dx == 1.0 && dy == 0.0) {
        return G2::MOVE_RIGHT;
    }
    return G2::REMAIN_STILL;
}


size_t MovementTypes::GetActionID(const PathVector &path)
{
    return GetActionID(path.X(), path.Y());

}
std::string MovementTypes::GetActionName(double dx, double dy)
{
    if (dx == 0.0 && dy == -1.0) {
        return "w";
    }
    if (dx == 0.0 && dy == 1.0) {
        return "s";
    }
    if (dx == -1.0 && dy == 0.0) {
        return "a";
    }
    if (dx == 1.0 && dy == 0.0) {
        return "d";
    }
    return "stay";

}
std::string MovementTypes::GetActionName(const PathVector &path)
{
    return GetActionName(path.X(), path.Y());
}
}
