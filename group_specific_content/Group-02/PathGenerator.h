/**
 * @file PathGenerator.h
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
#include <unordered_set>
#include <vector>
#include <memory>
#include "../../source/core/WorldPosition.hpp"


namespace cse498 {
class WorldPath;
class AgentAbility;
struct PathRequest;
class WorldGrid;
class ANode;

class PathGenerator
{
private:

    const static int MAX_SEARCH_DISTANCE = 300;

    enum class PathFlag
    {
        Skip, /// Skip is a fast efficient flag that skips over points it can't get to creating a path based on remaining points
        Expand /// Slower but creates better approximation of the original chosen shape if points are unavailable
    };
    static std::vector<WorldPosition> AStarSearch(const WorldPosition &from,
                                                  const WorldPosition &to,
                                                  const PathRequest &request);
    static double EuclideanDistance(const WorldPosition& from, const WorldPosition& to);
    /**
     * Finds if the node can be traveled to for direct surroundings (looking if corner spaces are valid)
     * @param from
     * @param dir
     * @param request
     * @return true if it can be traveled in that direction
     */
    static bool IsTravelable(const WorldPosition &from, const PathVector &dir, const PathRequest&);

public:
    /**
     *
     * @param agent_pos - orients the resulting list of points so the first is the shortest path to this point
     * @param circ_center - center point for the circle
     * @param circ_radius - radius for the circle
     * @param request - request information to make paths
     * @return list of points with starting point closest to agent_pos
     */
    static std::optional<WorldPath> CreateCircularPath(const WorldPosition& agent_pos, const WorldPosition& circ_center,
                                                       double circ_radius, const PathRequest& request, PathFlag flag = PathFlag::Skip);
    static std::optional<WorldPath> CreateRectangularPath(const WorldPosition& agent_pos, const WorldPosition& bot_left,
                                                          const WorldPosition& top_right, const PathRequest& request);
    static std::optional<WorldPath> CreateShortestPath(const WorldPosition& start, const WorldPosition& end, const PathRequest& request);
    /**
     * This is a strict function. If any tiles are blocked then it won't return anything. Only works if all free
     * @param start - start position
     * @param end
     * @param request
     * @return
     */
    static std::optional<WorldPath> CreateManhattanPath(const WorldPosition& start, const WorldPosition& end, const PathRequest& request);

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




}

#endif //SPRING2026_COMPANYA_GROUP_SPECIFIC_CONTENT_GROUP_02_PATHGENERATOR_H
