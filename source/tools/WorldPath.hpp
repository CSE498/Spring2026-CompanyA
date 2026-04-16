/**
 * @file WorldPath.hpp
 * @author Luke Antone
 *
 * WorldPath stores an ordered sequence of 2D positions for an agent to follow.
 * This is an initial milestone implementation and is expected to evolve.
 */

#ifndef SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H
#define SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H

#include <algorithm>
#include <cstddef>
#include <iosfwd>
#include <optional>
#include <ranges>
#include <span>
#include <utility>
#include <vector>
#include "../core/WorldPosition.hpp"

namespace cse498 {

class WorldPath {
public:
    WorldPath() = default;

    /**
     * Constructs a path from an existing span of world positions.
     */
    explicit WorldPath(std::span<const WorldPosition> path);

    /**
     * Removes all points from the path.
     */
    void Clear();

    /**
     * Adds a point to the end of the path.
     */
    void AddPoint(const WorldPosition& p);

    /**
     * Returns the number of points in the path.
     */
    std::size_t Size() const;

    /**
     * Returns true if the path contains no points.
     */
    bool Empty() const;

    /**
     * Returns the point at the given index.
     */
    const WorldPosition& At(std::size_t index) const;

    /**
     * Returns a read-only span view of the path's points.
     */
    std::span<const WorldPosition> Span() const noexcept;

    /**
     * Returns the total length of the path.
     */
    double Length() const;

    /**
     * Returns true if the path intersects itself.
     */
    bool SelfIntersects() const;

    /**
     * Returns indices of the two furthest points in the path.
     */
    std::optional<std::pair<std::size_t, std::size_t>> FurthestPointPair() const;

    /**
     * Reverses the world path in place.
     */
    WorldPath& Reverse() noexcept {
        std::ranges::reverse(mPoints);
        return *this;
    }

    /**
     * Appends points from another path to this path.
     */
    WorldPath& Extend(const WorldPath& other);

    bool operator==(const WorldPath&) const = default;

    friend std::ostream& operator<<(std::ostream& os, const WorldPath& path);

private:
    std::vector<WorldPosition> mPoints;
};

} // namespace cse498

#endif // SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H
