/**
 * @file MovementTypes.cpp
 * @author Logan Rimarcik, Jacob
 */

#include "MovementTypes.hpp"

#include "../../Worlds/DemoG2/WorldActions.hpp"
#include "../../tools/PathVector.hpp"


namespace cse498 {


std::size_t MovementTypes::GetActionID(double dx, double dy) {
    if (Approx(dx, 0.0) && Approx(dy, -1.0)) {
        return WorldActions::MOVE_UP;
    }
    if (Approx(dx, 0.0) && Approx(dy, 1.0)) {
        return WorldActions::MOVE_DOWN;
    }
    if (Approx(dx, -1.0) && Approx(dy, 0.0)) {
        return WorldActions::MOVE_LEFT;
    }
    if (Approx(dx, 1.0) && Approx(dy, 0.0)) {
        return WorldActions::MOVE_RIGHT;
    }
    return WorldActions::REMAIN_STILL;
}


size_t MovementTypes::GetActionID(const PathVector& path) { return GetActionID(path.X(), path.Y()); }
std::string MovementTypes::GetActionName(double dx, double dy) {
    if (Approx(dx, 0.0) && Approx(dy, -1.0)) {
        return WorldActions::MOVE_UP_STRING;
    }
    if (Approx(dx, 0.0) && Approx(dy, 1.0)) {
        return WorldActions::MOVE_DOWN_STRING;
    }
    if (Approx(dx, -1.0) && Approx(dy, 0.0)) {
        return WorldActions::MOVE_LEFT_STRING;
    }
    if (Approx(dx, 1.0) && Approx(dy, 0.0)) {
        return WorldActions::MOVE_RIGHT_STRING;
    }
    return WorldActions::REMAIN_STILL_STRING;
}
std::string MovementTypes::GetActionName(const PathVector& path) { return GetActionName(path.X(), path.Y()); }
} // namespace cse498
