/**
 * @file PathVector.cpp
 * @author lrima
 */

#include "PathVector.hpp"
#include "../core/WorldPosition.hpp"

namespace cse498
{


PathVector & PathVector::Rotate(double angle)
{
    // radians rotation CCW -- classic matrix rotation from LA
    auto temp_mx = mX;
    mX = std::cos(angle) * mX - std::sin(angle) * mY;
    mY = std::sin(angle) * temp_mx + std::cos(angle) * mY;

    return *this;
}


}
