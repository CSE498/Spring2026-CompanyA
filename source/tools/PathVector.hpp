/**
 * @file PathVector.h
 * @author lrima
 *
 *
 */

#pragma once



#include <cmath>
#include <string>

#include "../core/WorldPosition.hpp"

namespace cse498
{
class WorldPosition;

/**
 * PathVector is a directional vector that is in charge of
 * 1. Movement (You have a point a --> point b scaled by delta time and speed of the character)
 * 2. Calculations for collisions (you can test with vectors if the distance between two vectors are too close = collision)
 *
 */
class PathVector
{
private:
    double mX = 0.0;
    double mY = 0.0;


public:

    /**
     * Creates a path vector pointing in the direction of 'to'
     * Example: (0,0) --> (1,1) then resulting vector is 1,1
     * @param from
     * @param to
     */
    constexpr PathVector(const WorldPosition& from, const WorldPosition& to)
    {
        mX = to.X() - from.X();
        mY = to.Y() - from.Y();
    }
    /**
     * Constructor to make a vector directly. Put the magnitudes of x and y parts
     * @param x - x part
     * @param y - y part
     */
    constexpr PathVector(double x, double y) : mX(x), mY(y) {}
    constexpr PathVector operator+(const PathVector& rhs) const
    {
        return {mX + rhs.mX, mY + rhs.mY};
    }
    constexpr PathVector operator-(const PathVector& rhs) const
    {
        return {mX - rhs.mX, mY - rhs.mY};
    }
    constexpr bool operator==(const PathVector& rhs) const
    {
        return std::abs(mX - rhs.mX) < 1e-6 && std::abs(mY - rhs.mY) < 1e-6;
    }
    /// Classic dot project
    [[nodiscard]] constexpr double Dot(const PathVector& second) const { return mX * second.mX + mY * second.mY; }
    /// Hadamard Product
    [[nodiscard]] constexpr PathVector Mult(const PathVector& second) const { return {mX * second.mX, mY * second.mY}; }
    [[nodiscard]] constexpr PathVector Mult(const double x, const double y) const { return {mX * x, mY * y}; }

    // Operations:
    /**
     * Normalizes the vector IN-PLACE so magnitude is approx 1
     * @return modified in place vector
     */
    PathVector& Normalize()
    {
        double mag = GetMagnitude();
        mX /= mag;
        mY /= mag;
        return *this;
    }
    /**
     * IN-PLACE operation to scale the vector
     * @param scale_val scale factor for the vector. Changes x,y by this amount so mag --> mag * scale_val
     * @return scaled vector in place.
     */
    constexpr PathVector& Scale(double scale_val)
    {
        mX *= scale_val;
        mY *= scale_val;

        return *this;
    }



    [[nodiscard]] constexpr double X() const { return mX; }
    [[nodiscard]] constexpr double Y() const { return mY; }
    // In C++26 can be made constexpr but not to confuse anyone. It is currently not capable
    [[nodiscard]] double GetMagnitude() const { return std::sqrt(mX * mX + mY * mY); }
    /**
     * Gives angle with respect to horizontal axis in normal Euclidean calculations
     * @return angle CCW from x-axis
     */
    [[nodiscard]] double GetAngle() const { return std::atan2(mY, mX); }
    /**
     * rotates the path vector with respect to its start position counterclockwise by angle
     * not possible for constexpr until c++26 though
     * @param angle - angle of rotation CCW radians
     * @return in place modification of vector position
     */
    PathVector& Rotate(double angle);

    /**
     * Projects argument onto calling object returning new position of where the projection lands
     * @param this_onto_that whatever is put here is projected onto the object calling this function.
     * @return
     */
    [[nodiscard]] constexpr PathVector Project(const WorldPosition& this_onto_that) const
    {
        return Project(PathVector(this_onto_that.X(), this_onto_that.Y()));
    }
    [[nodiscard]] constexpr PathVector Project(const PathVector& this_onto_that) const
    {
        // project input onto "this"
        PathVector result = *this;
        double top = this_onto_that.Dot(*this);
        double bottom = this->Dot(*this);
        result.Scale(top / bottom);
        return result;
    }

};

inline WorldPosition operator+(const WorldPosition& p, const PathVector& v) { return {p.X() + v.X(), p.Y() + v.Y()}; }
inline WorldPosition operator-(const WorldPosition& p, const PathVector& v) { return {p.X() - v.X(), p.Y() - v.Y()}; }
inline PathVector operator-(const WorldPosition& p, const WorldPosition& pos) { return {p.X() - pos.X(), p.Y() - pos.Y()}; }

}




