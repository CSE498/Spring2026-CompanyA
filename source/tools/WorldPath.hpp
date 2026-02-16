/**
* @file WorldPath.h
 * @author Luke Antone
 *
 * WorldPath stores an ordered sequence of 2D positions for an agent to follow.
 * This is an initial milestone implementation and is expected to evolve.
 */

#ifndef SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H
#define SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H

#include <cstddef>
#include <iosfwd>
#include <algorithm>
#include <optional>
#include <span>
#include <utility>
#include <vector>
#include "../core/WorldPosition.hpp"

namespace cse498
{

    class WorldPath
    {
    public:
        WorldPath() = default;
        explicit WorldPath(std::span<const WorldPosition> path);

        void Clear();
        void AddPoint(const WorldPosition& p);

        std::size_t Size() const;
        bool Empty() const;

        const WorldPosition& At(std::size_t index) const;

        // Direct read-only access (legacy/compat).
        const std::vector<WorldPosition>& Points() const;

        // Preferred read-only view that does not expose the storage type.
        std::span<const WorldPosition> Span() const noexcept;

        double Length() const;

        bool SelfIntersects() const;

        // Returns indices of the two furthest points in the path.
        std::optional<std::pair<std::size_t, std::size_t>> FurthestPointPair() const;
        /**
         * Reverses the world path in place.
         */
        WorldPath& reverse()
        {
            std::ranges::reverse(mPoints);
            return *this;
        }


        bool operator==(const WorldPath&) const = default;
        friend std::ostream& operator<<(std::ostream& os, const WorldPath& path);
        WorldPath& extend(const WorldPath& other);
    private:
        std::vector<WorldPosition> mPoints;
    };

};



#endif // SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H
