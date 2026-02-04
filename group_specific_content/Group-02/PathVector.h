/**
 * @file PathVector.h
 * @author lrima
 *
 *
 */

#ifndef SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHVECTOR_H
#define SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHVECTOR_H


#include <cmath>

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
    double x = 0.0;
    double y = 0.0;


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
    PathVector(double x, double y) : x(x), y(y) {}
    PathVector operator+(const PathVector& rhs) const;
    PathVector operator-(const PathVector& rhs) const;
    /// Classic dot product
    PathVector operator*(const PathVector& rhs) const;

    [[nodiscard]] double X() const { return x; }
    [[nodiscard]] double Y() const { return y; }
    [[nodiscard]] double getMagnitude() const { return std::sqrt(std::pow(x, 2) + std::pow(y,2)); }
};


}



#endif //SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHVECTOR_H
