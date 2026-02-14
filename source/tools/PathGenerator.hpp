/**
 * @file PathGenerator.hpp
 * @author lrima
 *
 * This does path generation to a goal via A* and tile structure only in a really inefficient way compared to the
 * alternative methods. This file should be updated if used once we know the Tile System that our world uses
 * and better questions can be asked to the world grid.
 * TODO: Change idea: shortest path: use a vector from a --> b and ask if it goes through a tile of blocks
 * TODO cont: use Besenham's line algo or something to follow the line to a--> b and record all tiles it went through
 * TODO find if that intersected a tile block and ask the tile block to give you its corners then do A* on those corners
 * TODO to the final goal. Straight lines are the shortest so use A* on corners of obstacles.
 *
 * Note:
 * There are about 100 different ways to make this structure and the hardest part is that I don't know
 * what I'm allowed to change at this point and there is so much information I wish I had about the world's structure
 * and defining blocks of walls together as a range then asking the world so many things to do with line intersection
 * with walls and where corners of a block of walls is located.
 *
 * The way I decided is easiest least efficient and not particularly what I wanted but I have yet to know what I truly
 * want because we have yet to design what it is that we are doing. This will have to be changed when teams start
 * working together.
 */

#ifndef SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHGENERATOR_H
#define SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHGENERATOR_H


#include <span>
#include <optional>
#include <utility>
#include <queue>
#include <unordered_set>
#include <vector>
#include <memory>
#include "../core/WorldGrid.hpp"
#include "../core/WorldPosition.hpp"
#include "WorldPath.hpp"

namespace cse498 {
struct CirclePath;
class AgentAbility;
struct PathRequest;
struct ANode;
enum class PathFlag
{
    Skip, /// Skip is a fast efficient flag that skips over points it can't get to creating a path based on remaining points
    Expand /// Slower but creates better approximation of the original chosen shape if points are unavailable
};
enum class CircleDirectionFlag
{
    CW,
    CCW
};

/**
 * Generates paths for anything.
 */
class PathGenerator
{
private:

    static constexpr int MAX_SEARCH_DISTANCE = 300;
    static constexpr double CIRCLE_STEP_SIZE = 1;

    static constexpr double EuclideanDistance(const WorldPosition& p1, const WorldPosition& p2)
    {
        const double x_x = p1.X() - p2.X();
        const double y_y = p1.Y() - p2.Y();
        return std::sqrt(x_x * x_x + y_y * y_y);
    }

    /**
     * Return Struct for 2 values.
     */
    struct CircleTravel
    {
        bool possible;
        std::vector<WorldPosition> path;
    };

    /**
     * A heuristic for determining proximity to multiple goal positions.
     * This is for having an agent try to achieve any of the positions as sufficient to start path.
     */
    struct MultiGoalHeuristic
    {
        std::unordered_set<WorldPosition> goals;
        double operator()(const WorldPosition& p1) const
        {
            double best = std::numeric_limits<double>::infinity();
            for (const auto& each : goals)
            {
                best = std::min(best, EuclideanDistance(p1, each));
            }
            return best;
        }
        bool equals(const WorldPosition& p1) const
        {
            assert(p1.IsValid()); // I think this is always true? Let Logan Rimarcik know if this fails
            return goals.contains(round(p1));
        }
    };

    /**
     * Represents a heuristic function that calculates the distance between a given position and a target goal.
     * Also provides functionality to check if two positions are approximately equal.
     */
    struct SingleGoalHeuristic
    {
        const WorldPosition goal;
        double operator()(const WorldPosition& p1) const
        {
            return EuclideanDistance(p1, goal);
        }
        [[nodiscard]] bool equals(const WorldPosition& p1) const
        {
            if (!goal.IsValid() || !p1.IsValid())
                return false;
            return round(goal) == round(p1);
        }
    };



    /**
     * Semi-Classic A* with some more options
     * Starts at "from" then moves in units of 1 (hardcoded) with diagonals (1,1) length sqrt(2) to find the ending TILE
     * The end TILE IS NOT the precise point in GOAL defined by "h" -- to do so just push it back onto the returned val
     * @param from - start pos
     * @param request - other necessary info
     * @param h - contains the goal and heuristic search measures (contains final "goal" 'to' position)
     * @param max_search_dist - max search distance to check so it doesn't run forever
     * @param closest - if true then returns path to the closest point it was able to get to relative to the goal
     *                  otherwise returns nothing if it wasn't able to get to the goal completely
     * @return
     */
    template <typename Heu>
    static std::vector<WorldPosition> AStarSearch(const WorldPosition &from,
                                                  const PathRequest &request,
                                                  Heu h,
                                                  double max_search_dist = MAX_SEARCH_DISTANCE,
                                                  bool closest = false);
    static std::vector<WorldPosition> AStarReconstruction(const std::shared_ptr<ANode>& node);

    /**
     * Creates the next point circle step size in the direction on a circle
     * @param start - start pos that lies on the circle
     * @param center - center of circle
     * @param radius - radius of circle
     * @param flag - default CW, optional CCW
     * @return - the next point CIRCLE UNITS away from the previous in straight line distance - could change obv
     */
    static WorldPosition findNextCirclePos(const WorldPosition &start, const WorldPosition& center, double radius,
        CircleDirectionFlag flag = CircleDirectionFlag::CW);

    /**
     * Finds if the node can be traveled to for direct surroundings (looking if corner spaces are valid)
     * @param from - original position
     * @param dir - direction vector to the new position (add to from to get new pos)
     * @param request - request info for the path
     * @return true if it can be traveled in that direction
     */
    static bool IsTravelable(const WorldPosition &from, const PathVector &dir, const PathRequest& request);
    static CircleTravel IsTravelableCircle(const WorldPosition &from, const WorldPosition &to, const PathRequest &request);
    /**
     * Determines if a point comes before another point on a circle during generation.
     * @param test_pt - point to test or ask the question about
     * @param relative_pt - relative to this point. this is the static point compared to
     * @param center - circle center
     * @param flag - direction of the circle generation
     * @return T/F
     */
    static bool IsPointBefore(const WorldPosition& test_pt, const WorldPosition& relative_pt, const WorldPosition& center, CircleDirectionFlag flag);
    /**
     * Finds only a path around the circle given that the start is on the circle whic which is a requirement
     * Start can be determined a lot of ways refer to FindCircle function to see how
     * @param start Should be a point on the circle. Can be a double
     * @param circ_center - center of circle
     * @param circ_radius - radius of circle
     * @param request - request info
     * @param flag - flag for how to generate circle, expand pushes agent outwards, skip chops the circle if not available
     * @param circle_flag - CCW or CW
     * @return
     */
    static WorldPath makeCircle(const WorldPosition& start,
                                const WorldPosition& circ_center,
                                double circ_radius,
                                const PathRequest& request,
                                PathFlag flag,
                                CircleDirectionFlag circle_flag);

public:

    /**
     * This isn't *fully* functional intentionally because the correct world doesn't exist yet for this.
     * This is a _rough_ class ensuring the key components are done for adaptation:
     * TODO: Update upon new world implementation
     * @param agent_pos - orients the resulting list of points so the first is the shortest path to this point
     * @param circ_center - center point for the circle
     * @param circ_radius - radius for the circle
     * @param request - request information to make paths
     * @param flag - type of generation whether skipping points or trying to get close to those points
     * @param circle_flag - generation direction
     * @return list of points with starting point closest to agent_pos
     */
    static std::optional<CirclePath> FindCircularPath(const WorldPosition& agent_pos,
                                                     const WorldPosition& circ_center,
                                                     double circ_radius,
                                                     const PathRequest& request,
                                                     PathFlag flag = PathFlag::Skip,
                                                     CircleDirectionFlag circle_flag = CircleDirectionFlag::CW);


    /**
     * Makes a loop from bot left to top right passed back separately from the provided path to get to the loop
     * It is the shortest path to any position in the loop
     * @param agent_pos - agent position currently
     * @param bot_left - loops bottom left corner
     * @param top_right - loops top right corner
     * @param request - other parameters to watch out for.
     * @return
     */
    static std::optional<CirclePath> FindRectangularLoopPath(const WorldPosition& agent_pos, const WorldPosition& bot_left,
                                                            const WorldPosition& top_right, const PathRequest& request,
                                                            CircleDirectionFlag flag = CircleDirectionFlag::CW);
    static std::optional<std::vector<WorldPosition>> MakeRectangleLoop(const WorldPosition &bot_left,
                                                              const WorldPosition &top_right,
                                                              const PathRequest &request,
                                                              CircleDirectionFlag flag);
    /**
     * Creates Shortest path from start to end progressing in step counts of 1 and ending on same tile found
     * @param start - start position of path
     * @param end - end position of path (relative to the tile)
     * @param request - request information
     * @return
     */
    static std::optional<WorldPath> FindShortestPath(const WorldPosition& start, const WorldPosition& end, const PathRequest& request);
    /**
     * This is a strict function. If any tiles are blocked then it won't return anything. Only works if all free
     * @param start - start position
     * @param end
     * @param request
     * @param flag - generation direction
     * @return
     */
    static std::optional<WorldPath> FindManhattanPath(const WorldPosition& start, const WorldPosition& end, const PathRequest& request,
        CircleDirectionFlag flag = CircleDirectionFlag::CW);

};





/*
 * ------------------------------------------------------------------------------------------------------
 * Extra helper classes (PathRequest) and Enum class:
 * ------------------------------------------------------------------------------------------------------
 */

/**
 * These are the nodes used for A* search
 */
struct ANode
{
    WorldPosition pos;
    double g = 0; // cost from start position (total distance traveled)
    double f = 0; // g + h
    /// This is for reconstruction. Not sure what is best here maybe std::optional<WorldPosition> and just copy store it
    std::shared_ptr<ANode> prev;

};

/**
 * comparison operaor for the priority queue
 */
struct ANodeCompare
{
    bool operator()(const std::shared_ptr<ANode>& a, const std::shared_ptr<ANode>& b) const
    {
        return a->f > b->f;
    }
};

struct PathRequest
{
public:
    /// the points known to complete a path between and ensure valid
    std::unordered_set<WorldPosition> avoid_tiles;
    const AgentAbility& ability;
    const WorldGrid& world_grid;

    /**
     *
     * @param avoid_tiles pass in {} if there are no such tiles. Otherwise treated as
     * list of points to avoid default to entire tile. so given (1,1) == [0.5, 1.49) in x,y
     * @param ability  reference as this path is bound and only valid for the created agent so its abilities follow
     * @param world_grid
     */
    PathRequest(std::unordered_set<WorldPosition> avoid_tiles,
                const AgentAbility& ability, const WorldGrid& world_grid) : avoid_tiles(std::move(avoid_tiles)), ability(ability),
                                                                            world_grid(world_grid)
    {
    }
};

/**
 * Return struct
 */
struct CirclePath
{
    WorldPath path_to_circle;
    WorldPath circle_path;
};



/*
 * Templated Functions for PathGenerator
 */



/*
 * A*, we have a priority queue sorted by distances and find the path from A --> G and return the full length of the path
 *
 */
template <typename Heu>
std::vector<WorldPosition> PathGenerator::AStarSearch(const WorldPosition &from,
                                                      const PathRequest &request,
                                                      Heu h,
                                                      const double max_search_dist, const bool closest)
{

    if (!request.world_grid.IsWalkable(from))
        return {};

    const auto directions = std::to_array<PathVector>({
        {1, 0}, {0, 1}, {-1, 0}, {0, -1}, {1, 1}, {-1, 1}, {1, -1}, {-1, -1}
    });
    std::priority_queue<std::shared_ptr<ANode>, std::vector<std::shared_ptr<ANode>>, ANodeCompare> pq;
    std::unordered_set<WorldPosition> visited = {};
    double closest_distance = h(from); // Does heuristic calc to check distances
    std::shared_ptr<ANode> closest_node;

    pq.push(std::make_shared<ANode>(from, 0, 0, nullptr));
    visited.insert({from.X(), from.Y()});
    while (!pq.empty())
    {
        auto node = pq.top();
        pq.pop();
        // Just check if the node is too far away from the goal and this is our current best estimate then stop looking
        if (node->f > max_search_dist) // Could change this to distance traveled instead
        {
            if (closest && closest_node != nullptr)
            {
                return AStarReconstruction(closest_node);
            }
            return {};
        }
        if (closest)
        {
            double close_test = h(node->pos);
            if (close_test < closest_distance)
            {
                closest_distance = close_test;
                closest_node = node;
            }
        }

        if (h.equals(node->pos)) // Checks
        {
            return AStarReconstruction(node);
        }
        // Otherwise continue adding to priority queue
        for (const auto &dir : directions)
        {
            WorldPosition neighbor = node->pos + dir;
            // if the neighbor is not a valid walking tile then skip it
            // Travelable checks other conditions (banned list, possible for immediate movement) + Walkablility
            if (!IsTravelable(node->pos, dir, request))
                continue;

            // Order of arguments is Node, g, f, prev node
            double g = node->g + dir.getMagnitude();
            if (!visited.contains(neighbor))
            {
                pq.push(std::make_shared<ANode>(neighbor, g, g + h(round(neighbor)), node));
                visited.insert(neighbor);
            }
        }
    }
    // Not possible to reach the goal.
    return {};
}

}

#endif //SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHGENERATOR_H
