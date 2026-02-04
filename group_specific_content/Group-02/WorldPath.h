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
#include <optional>
#include <span>
#include <utility>
#include <vector>
#include "../../source/core/WorldPosition.hpp"

namespace cse498
{

class WorldPath
{
public:
    WorldPath() = default;
    explicit WorldPath(std::span<const WorldPosition> path);

    void Clear();
    void AddPoint(const WorldPosition& p);

    [[nodiscard]] std::size_t Size() const;
    [[nodiscard]] bool Empty() const;

    [[nodiscard]] const WorldPosition& At(std::size_t index) const;
    [[nodiscard]] const std::vector<WorldPosition>& Points() const;

    [[nodiscard]] double Length() const;
    [[nodiscard]] bool SelfIntersects() const;

    // Returns indices of the two furthest points in the path.
    [[nodiscard]] std::optional<std::pair<std::size_t, std::size_t>> FurthestPointPair() const;

private:
    std::vector<WorldPosition> mPoints;
};

std::ostream& operator<<(std::ostream& os, const WorldPath& path);


}


#endif //SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_WORLDPATH_H
