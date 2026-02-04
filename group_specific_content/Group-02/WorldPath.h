/**
 * @file WorldPath.h
 * @author lrima
 *
 *
 */

#ifndef SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H
#define SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H

#include <span>
#include <iostream>
#include <vector>
#include "../../source/core/WorldPosition.hpp"

namespace cse498
{

class WorldPath
{
private:
    std::vector<WorldPosition> points;

public:

    explicit WorldPath(std::span<WorldPosition> path);

    friend std::ostream& operator<<(std::ostream& os, const WorldPath& path);
};

inline std::ostream& operator<<(std::ostream& os, const cse498::WorldPath& path)
{
    for (std::size_t i = 0; i < path.points.size(); i++)
    {
        os << "Point " << i << " -- X: " << path.points[i].X() << " Y: " << path.points[i].Y() << '\n';
    }
    return os;
}


}


#endif //SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H
