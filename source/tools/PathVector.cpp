/**
 * @file PathVector.cpp
 * @author lrima
 */

#include "PathVector.hpp"
#include "../core/WorldPosition.hpp"

namespace cse498
{

PathVector::PathVector(const WorldPosition &from, const WorldPosition &to)
{
    mX = to.X() - from.X();
    mY = to.Y() - from.Y();
}
PathVector PathVector::operator+(const PathVector &rhs) const
{
    return {mX + rhs.mX, mY + rhs.mY};

}
PathVector PathVector::operator-(const PathVector &rhs) const
{
    return {mX - rhs.mX, mY - rhs.mY};
}

PathVector & PathVector::Rotate(double angle)
{
    // radians rotation CCW -- classic matrix rotation from LA
    auto temp_mx = mX;
    mX = std::cos(angle) * mX - std::sin(angle) * mY;
    mY = std::sin(angle) * temp_mx + std::cos(angle) * mY;

    return *this;
}

PathVector PathVector::Project(const WorldPosition &this_onto_that) const
{
    return Project(PathVector(this_onto_that.X(), this_onto_that.Y()));
}
PathVector PathVector::Project(const PathVector &this_onto_that) const
{
    // project input onto "this"
    PathVector result = *this;
    double top = this_onto_that.Dot(*this);
    double bottom = this->Dot(*this);
    result.Scale(top / bottom);
    return result;
}
}
