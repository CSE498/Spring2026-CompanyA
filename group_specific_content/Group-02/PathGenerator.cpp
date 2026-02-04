/**
 * @file PathGenerator.cpp
 * @author lrima
 */


#include <cassert>
#include <array>
#include <cmath>
#include <unordered_map>
#include <queue>
#include <unordered_set>
#include <algorithm>
#include "PathGenerator.h"
#include "WorldPath.h"
#include "../../source/core/WorldGrid.hpp"

// TODO: add middle points and blocked points into the mix.

namespace cse498
{

 double PathGenerator::EuclideanDistance(const WorldPosition &from, const WorldPosition &to)
{
     return std::sqrt(std::pow(from.X() - to.X(), 2) + std::pow(from.Y() - to.Y(), 2));
}


bool PathGenerator::IsTravelable(const WorldPosition& from, const PathVector& dir, const PathRequest& request)
{
     if (dir.X() == 0 || dir.Y() == 0)
         return true;
    // capable of being traveled within only this context. DOES NOT determine if walkable. This is just if it can be
    // walked to.
    // Example:
    /*      x_x
     *      x__
     *      _x_
     *      in this 3x3 the bottom left can't be traveled to because two blocks are cutting it off
     */
    WorldPosition test1 = from + dir * PathVector(1,0);
    WorldPosition test2 = from + dir * PathVector(0,1);
    return request.world_grid.IsWalkable(test1) && request.world_grid.IsWalkable(test2);
}

/*
 * A*, we have a priority queue sorted by distances and find the path from A --> G and return the full length of the path
 *
 */
std::vector<WorldPosition> PathGenerator::AStarSearch(const WorldPosition& from, const WorldPosition& to, const PathRequest& request)
{
    if (!request.world_grid.IsWalkable(from))
        return {};

    const auto directions = std::to_array<PathVector>({{1,0}, {0,1}, {-1,0}, {0,-1}, {1,1}, {-1,1}, {1,-1}, {-1,-1}});
    std::priority_queue<std::shared_ptr<ANode>, std::vector<std::shared_ptr<ANode>>, ANodeCompare> pq;
    std::unordered_set<WorldPosition, std::hash<WorldPosition>> visited = {};

    pq.push(std::make_shared<ANode>(from, 0,0, nullptr));
    visited.insert({1,1});
    while (!pq.empty())
    {
        auto node = pq.top();
        pq.pop();
        // Just check if the node is too far away from the goal and this is our current best estimate then stop looking
        if (node->f > MAX_SEARCH_DISTANCE)
            return {};

        if (node->pos == to)
        {
            std::vector<WorldPosition> result;
            // Reconstruct the path and return.
            result.push_back(node->pos);
            auto prev = node->prev;
            while (prev)
            {
                result.push_back(prev->pos);
                prev = prev->prev;
            }
            std::reverse(result.begin(), result.end());
            return result;
        }
        // Otherwise continue adding to priority queue
        for (const auto& dir : directions)
        {
            WorldPosition neighbor = node->pos + dir;
            // if the neighbor is not a valid walking tile then skip it
            if (!request.world_grid.IsWalkable(neighbor) || !IsTravelable(node->pos, dir, request))
                continue;

            // Order of arguments is Node, g, f, prev node
            double g = node->g + dir.getMagnitude();
            if (!visited.contains(neighbor))
                pq.push(std::make_shared<ANode>(neighbor, g, g + EuclideanDistance(neighbor, to), node));
            visited.insert(neighbor);
        }
    }
    // Not possible to reach the goal.
    return {};

}


std::optional<WorldPath> PathGenerator::CreateCircularPath(const WorldPosition &agent_pos,
                                                           const WorldPosition &circ_center,
                                                           double circ_radius,
                                                           const PathRequest &request, PathFlag flag)
{
    /*
     * Goal is to return a list of points that are 1 unit apart for the most part making a complete circle
     * The start of the circle is at the point nearest to the agent given that tile is reachable. Otherwise it is
     * the top of the circle. --> If top is unreachable then it will be the first point that appears that is reachable
     * from the top and going clockwise. If you want to go counterclockwise then the list is just reversed -- that
     * can be done later.
     */

    // Step 1: Find the starting point

    if (flag == PathFlag::Skip)
    {

    }


    return std::optional<WorldPath>();
}
std::optional<WorldPath> PathGenerator::CreateRectangularPath(const WorldPosition &agent_pos,
                                                              const WorldPosition &bot_left,
                                                              const WorldPosition &top_right,
                                                              const PathRequest &request)
{
    return std::optional<WorldPath>();
}
std::optional<WorldPath> PathGenerator::CreateShortestPath(const WorldPosition &start,
                                                           const WorldPosition &end,
                                                           const PathRequest &request)
{
    /*
    * We need to make the shortest path from start to end with certain other parameters.
    * We are going to do this a "bad" way of following 8 directions around the player and do A* search on that
    * result to the goal. This probably won't be that quick and can be made faster after more information
    * about the games structure is understood, and I can work/edit the right files.
    */
    // Step 1:
    auto result = AStarSearch(start, end, request);
    if (result.empty())
        return {};
    return WorldPath(result);

}
std::optional<WorldPath> PathGenerator::CreateManhattanPath(const WorldPosition &start,
                                                            const WorldPosition &end,
                                                            const PathRequest &request)
{
    return std::optional<WorldPath>();
}



}
