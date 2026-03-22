/**
 * @file PathVector.h
 * @author lrima
 *
 *
 */

#ifndef SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHVECTOR_H
#define SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHVECTOR_H


#include <cmath>
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
    PathVector(const WorldPosition& from, const WorldPosition& to);
    /**
     * Constructor to make a vector directly. Put the magnitudes of x and y parts
     * @param x - x part
     * @param y - y part
     */
    PathVector(double x, double y) : mX(x), mY(y) {}
    PathVector operator+(const PathVector& rhs) const;
    PathVector operator-(const PathVector& rhs) const;
    bool operator==(const PathVector& rhs) const
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
    constexpr PathVector& Normalize()
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

    /**
     * rotates the path vector with respect to its start position counterclockwise by angle
     * not possible for constexpr until c++26 though
     * @param angle - angle of rotation CCW radians
     * @return in place modification of vector position
     */
    PathVector& Rotate(double angle);

    [[nodiscard]] double X() const { return mX; }
    [[nodiscard]] double Y() const { return mY; }
    /// can't be constexpr because of sqrt but put it anyway since close to c++26
    [[nodiscard]] constexpr double GetMagnitude() const { return std::sqrt(mX * mX + mY * mY); }
    [[nodiscard]] constexpr double GetAngle() const { return std::atan2(mY, mX); }

    /**
     * Projects argument onto calling object returning new position of where the projection lands
     * @param this_onto_that whatever is put here is projected onto the object calling this function.
     * @return
     */
    [[nodiscard]] PathVector Project(const WorldPosition& this_onto_that) const;
    [[nodiscard]] PathVector Project(const PathVector& this_onto_that) const;

};


inline WorldPosition operator+(const WorldPosition& p, const PathVector& v) { return {p.X() + v.X(), p.Y() + v.Y()}; }
inline WorldPosition operator-(const WorldPosition& p, const PathVector& v) { return {p.X() - v.X(), p.Y() - v.Y()}; }
inline PathVector operator-(const WorldPosition& p, const WorldPosition& pos) { return {p.X() - pos.X(), p.Y() - pos.Y()}; }

}



#endif //SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHVECTOR_H
