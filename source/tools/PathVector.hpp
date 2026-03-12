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
    /// Classic dot project
    [[nodiscard]] double Dot(const PathVector& second) const
    { return mX * second.mX + mY * second.mY; }
    /// Hadamard Product
    [[nodiscard]] PathVector mult(const PathVector& second) const { return {mX * second.mX, mY * second.mY}; }
    [[nodiscard]] PathVector mult(const double x, const double y) const { return {mX * x, mY * y}; }

    // Operations:
    /**
     * Normalizes the vector IN-PLACE so magnitude is approx 1
     * @return modified in place vector
     */
    PathVector& normalize();
    /**
     * IN-PLACE operation to scale the vector
     * @param scale_val scale factor for the vector. Changes x,y by this amount so mag --> mag * scale_val
     * @return scaled vector in place.
     */
    PathVector& scale(double scale_val);

    /**
     * rotates the path vector with respect to its start position counterclockwise by angle
     * @param angle - angle of rotation CCW
     * @return in place modification of vector position
     */
    PathVector& rotate(double angle);

    [[nodiscard]] double X() const { return mX; }
    [[nodiscard]] double Y() const { return mY; }
    [[nodiscard]] double getMagnitude() const { return std::sqrt(mX * mX + mY * mY); }
    [[nodiscard]] double getAngle() const { return std::atan2(mY, mX); }

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
