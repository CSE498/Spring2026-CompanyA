/**
 * @file MovementTypes.cpp
 * @author Logan Rimarcik, Jacob
 */

#include "MovementTypes.hpp"

#include "Worlds/DemoSimpleWorldG2Actions.hpp"

namespace G2 = cse498::DemoSimpleWorldG2Actions;

namespace cse498
{

std::size_t MovementTypes::DeltaToMoveAction(double dx, double dy) {
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


}
