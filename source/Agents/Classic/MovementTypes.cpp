/**
 * @file MovementTypes.cpp
 * @author Logan Rimarcik, Jacob
 */

#include "MovementTypes.hpp"

#include "../../Worlds/DemoG2/WorldActions.hpp"
#include "../../tools/PathVector.hpp"

namespace cse498 {


std::size_t MovementTypes::GetActionID(double dx, double dy) {
    // CHeck for invalid including 0,0
    if (!CheckAndFix(dx,dy))
        return WorldActions::REMAIN_STILL;

    // handle  (0,1), (0,-1)
    if (Approx(dx, 0.0)) {
        if (Approx(dy, -1.0))
            return WorldActions::MOVE_UP;
        return WorldActions::MOVE_DOWN;
    }
    // Otherwise assume dy is 0
    // handle (1,0), (-1,0)
    if (Approx(dx, 1.0))
        return WorldActions::MOVE_RIGHT;
    return WorldActions::MOVE_LEFT;
}

size_t MovementTypes::GetActionID(const PathVector& path) { return GetActionID(path.X(), path.Y()); }

std::string MovementTypes::GetActionName(double dx, double dy) {
    // CHeck for invalid including 0,0
    if (!CheckAndFix(dx,dy))
        return WorldActions::REMAIN_STILL_STRING;

    // handle (0,0), (0,1), (0,-1)
    if (Approx(dx, 0.0)) {
        if (Approx(dy, -1.0))
            return WorldActions::MOVE_UP_STRING;
        return WorldActions::MOVE_DOWN_STRING;
    }
    // Otherwise assume dy is 0
    // handle (1,0), (-1,0)
    if (Approx(dx, 1.0))
        return WorldActions::MOVE_RIGHT_STRING;
    return WorldActions::MOVE_LEFT_STRING;
}
std::string MovementTypes::GetActionName(const PathVector& path) { return GetActionName(path.X(), path.Y()); }


bool MovementTypes::CheckAndFix(double x, double y)
{
    // checks if (1) or (-1) for both indicating it is diagonal movement or magnitude of vector is 1
    return (std::abs(std::abs(x) - 1) < EPS && std::abs(std::abs(y) - 1) < EPS) || std::abs(PathVector(x, y).GetMagnitude() - 1) < EPS;

}

} // namespace cse498

