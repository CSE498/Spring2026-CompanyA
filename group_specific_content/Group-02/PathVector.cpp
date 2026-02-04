/**
 * @file PathVector.cpp
 * @author lrima
 */

#include "PathVector.h"
#include "../../source/core/WorldPosition.hpp"

namespace cse498
{

PathVector::PathVector(const WorldPosition &from, const WorldPosition &to)
{
    x = to.X() - from.X();
    y = to.Y() - from.Y();
}
PathVector PathVector::operator+(const PathVector &rhs) const
{
    return {x + rhs.x, y + rhs.y};

}
PathVector PathVector::operator-(const PathVector &rhs) const
{
    return {x - rhs.x, y - rhs.y};
}
PathVector PathVector::operator*(const PathVector &rhs) const
{
    return {x * rhs.x, y * rhs.y};
}

}