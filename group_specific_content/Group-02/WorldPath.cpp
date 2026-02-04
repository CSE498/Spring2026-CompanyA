/**
 * @file WorldPath.cpp
 * @author Luke Antone
 *
 * WorldPath stores an ordered sequence of 2D positions for an agent to follow.
 * This is an initial milestone implementation and is expected to evolve.
 */

#include "WorldPath.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <ostream>

namespace cse498
{

namespace
{
// Straight-line distance between two world positions.
[[nodiscard]] double Dist(const WorldPosition& a, const WorldPosition& b)
{
    const double dx = a.X() - b.X();
    const double dy = a.Y() - b.Y();
    return std::sqrt(dx * dx + dy * dy);
}

// Small 2D vector used for intersection math (kept local to this file).
struct Vec2
{
    double x = 0.0;
    double y = 0.0;
};

// Convert a WorldPosition into a plain 2D vector.
[[nodiscard]] Vec2 ToVec2(const WorldPosition& p) { return {p.X(), p.Y()}; }

// 2D cross product (scalar).
[[nodiscard]] double Cross(const Vec2& a, const Vec2& b) { return a.x * b.y - a.y * b.x; }

// Vector subtraction.
[[nodiscard]] Vec2 Sub(const Vec2& a, const Vec2& b) { return {a.x - b.x, a.y - b.y}; }

// Floating-point comparison with a small tolerance.
[[nodiscard]] bool NearlyEqual(double a, double b, double eps = 1e-9)
{
    return std::abs(a - b) <= eps;
}

// True if p lies on the segment a-b (assuming collinearity).
[[nodiscard]] bool OnSegment(const Vec2& a, const Vec2& b, const Vec2& p)
{
    const double minx = std::min(a.x, b.x);
    const double maxx = std::max(a.x, b.x);
    const double miny = std::min(a.y, b.y);
    const double maxy = std::max(a.y, b.y);

    return p.x >= minx - 1e-9 && p.x <= maxx + 1e-9 && p.y >= miny - 1e-9 && p.y <= maxy + 1e-9;
}

// Segment/segment intersection test for path self-intersection checks.
[[nodiscard]] bool SegmentsIntersect(const WorldPosition& a0,
                                    const WorldPosition& a1,
                                    const WorldPosition& b0,
                                    const WorldPosition& b1)
{
    const Vec2 p = ToVec2(a0);
    const Vec2 r = Sub(ToVec2(a1), p);
    const Vec2 q = ToVec2(b0);
    const Vec2 s = Sub(ToVec2(b1), q);

    const double rxs = Cross(r, s);
    const double q_pxs = Cross(Sub(q, p), r);

    if (NearlyEqual(rxs, 0.0) && NearlyEqual(q_pxs, 0.0))
    {
        // Collinear: check overlap via endpoint containment.
        return OnSegment(p, ToVec2(a1), q) || OnSegment(p, ToVec2(a1), ToVec2(b1)) ||
               OnSegment(q, ToVec2(b1), p) || OnSegment(q, ToVec2(b1), ToVec2(a1));
    }

    if (NearlyEqual(rxs, 0.0) && !NearlyEqual(q_pxs, 0.0))
        return false; // Parallel non-intersecting.

    const Vec2 q_p = Sub(q, p);
    const double t = Cross(q_p, s) / rxs;
    const double u = Cross(q_p, r) / rxs;

    return t >= -1e-9 && t <= 1.0 + 1e-9 && u >= -1e-9 && u <= 1.0 + 1e-9;
}
} // namespace

// Build a path from an existing sequence of points.
WorldPath::WorldPath(std::span<const WorldPosition> path) : mPoints(path.begin(), path.end()) {}

// Remove all points from the path.
void WorldPath::Clear() { mPoints.clear(); }

// Append a point to the end of the path.
void WorldPath::AddPoint(const WorldPosition& p) { mPoints.push_back(p); }

// Number of points currently in the path.
std::size_t WorldPath::Size() const { return mPoints.size(); }

// Convenience: true if the path has no points.
bool WorldPath::Empty() const { return mPoints.empty(); }

// Bounds-checked access to a point by index.
const WorldPosition& WorldPath::At(std::size_t index) const { return mPoints.at(index); }

// Direct read-only access to the underlying points container.
const std::vector<WorldPosition>& WorldPath::Points() const { return mPoints; }

// Total length computed by summing distances between consecutive points.
double WorldPath::Length() const
{
    if (mPoints.size() < 2)
        return 0.0;

    double total = 0.0;
    for (std::size_t i = 1; i < mPoints.size(); ++i)
        total += Dist(mPoints[i - 1], mPoints[i]);

    return total;
}

// Returns true if any non-adjacent segments in the path cross.
bool WorldPath::SelfIntersects() const
{
    if (mPoints.size() < 4)
        return false;

    for (std::size_t i = 1; i < mPoints.size(); ++i)
    {
        const WorldPosition& a0 = mPoints[i - 1];
        const WorldPosition& a1 = mPoints[i];

        for (std::size_t j = i + 2; j < mPoints.size(); ++j)
        {
            const WorldPosition& b0 = mPoints[j - 1];
            const WorldPosition& b1 = mPoints[j];

            if (SegmentsIntersect(a0, a1, b0, b1))
                return true;
        }
    }

    return false;
}

// Indices of the two points that are farthest apart (O(n^2)).
std::optional<std::pair<std::size_t, std::size_t>> WorldPath::FurthestPointPair() const
{
    if (mPoints.size() < 2)
        return std::nullopt;

    double best = -std::numeric_limits<double>::infinity();
    std::pair<std::size_t, std::size_t> bestPair{0u, 1u};

    for (std::size_t i = 0; i < mPoints.size(); ++i)
    {
        for (std::size_t j = i + 1; j < mPoints.size(); ++j)
        {
            const double d = Dist(mPoints[i], mPoints[j]);
            if (d > best)
            {
                best = d;
                bestPair = {i, j};
            }
        }
    }

    return bestPair;
}

// Compact debug print (count + total length).
std::ostream& operator<<(std::ostream& os, const WorldPath& path)
{
    os << "WorldPath{points=" << path.Size() << ", length=" << path.Length() << "}";
    return os;
}

} // namespace cse498
