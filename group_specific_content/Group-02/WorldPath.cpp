/**
 * @file WorldPath.cpp
 * @author lrima
 */

#include <ostream>
#include "WorldPath.h"
#include "PathGenerator.h"

namespace cse498
{
WorldPath::WorldPath(std::span<WorldPosition> path) : points(path.begin(), path.end())
{

}
}


