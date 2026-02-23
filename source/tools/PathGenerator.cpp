/**
 * @file PathGenerator.cpp
 * @author Logan Rimarcik
 */

#include <cassert>
#include <array>
#include <cmath>
#include <unordered_map>
#include <unordered_set>
#include <algorithm>
#include "PathGenerator.hpp"
#include "WorldPath.hpp"
#include "../core/WorldGrid.hpp"

namespace cse498
{
bool PathGenerator::IsTravelable(const WorldPosition &from, const PathVector &dir, const PathRequest &request)
{
    auto next_pos = from + dir;
    if (!request.world_grid.IsWalkable(next_pos) || request.avoid_tiles.contains(round(next_pos)))
        return false;
    if (dir.X() == 0 || dir.Y() == 0) // You can always move in the 4 cardinals given tile is valid
        return true;
    // capable of being traveled within only this context. DOES NOT determine if walkable. This is just if it can be
    // walked to.
    // Example:
    /*      x_x
     *      x__
     *      _x_
     *      in this 3x3 the bottom left can't be traveled to because two blocks are cutting it off
     */
    const WorldPosition test1 = from + dir.mult({1, 0});
    const WorldPosition test2 = from + dir.mult({0, 1});
    return request.world_grid.IsWalkable(test1) && request.world_grid.IsWalkable(test2);
}

PathGenerator::CircleTravel PathGenerator::IsTravelableCircle(const WorldPosition &from,
                                                              const WorldPosition &to,
                                                              const PathRequest &request)
{
    // find the direction of the 'to'
    const auto vec = PathVector(from, to);
    if (vec.getMagnitude() > 1 + 1e-6)
    {
        // Some point was skipped. We no longer know if we can travel there directly: Find weird path there instead
        auto pth = AStarSearch(from, request, SingleGoalHeuristic{to}, vec.getMagnitude() * 2, false);
        if (!pth.empty())
        {
            // it brings us to the right tile but we change the actual position on that tile to what we want.
            // This will be generally better though some spots that are longer but if we can get to the tile then we
            // can get to a specific position on that tile type of thing.
            // TODO: warning if the agent has hitbox > 0 then this may push agent into the wall to get to this position
            pth.pop_back();
            pth.push_back(to);

            return {true, pth};
        }
        return {false, pth};
    }

    return {IsTravelable(from, vec, request), {from, to}};
}
bool PathGenerator::IsPointBefore(const WorldPosition &test_pt,
                                  const WorldPosition &relative_pt,
                                  const WorldPosition &center,
                                  const CircleDirectionFlag flag)
{
    // Again clockwise = Counterclockwise and vise versa. Everything is opposite and it hurts my head
    // Assume both points are on the circle
    const PathVector center_to_test(test_pt, center);
    const PathVector center_to_relative(relative_pt, center);
    assert(std::abs(center_to_relative.getMagnitude() - center_to_test.getMagnitude()) < 1e-6);

    const PathVector test(test_pt.X() - center.X(), test_pt.Y() - center.Y());
    const PathVector relative(relative_pt.X() - center.X(), relative_pt.Y() - center.Y());

    // This determines if a point is before another point by comparing the angles they make.
    // The weird if statements are because of how the angle is returned from getAngle -- it returns in domain [-pi, pi]
    double test_angle = test.getAngle();
    double rel_angle = relative.getAngle();
    if (flag == CircleDirectionFlag::CW && rel_angle < 0 && test_angle > 0)
        rel_angle += 2 * std::numbers::pi;
    if (flag == CircleDirectionFlag::CCW && rel_angle > 0 && test_angle < 0)
        test_angle += 2 * std::numbers::pi;
    const double result = rel_angle - test_angle;
    if (std::abs(result) > std::numbers::pi)
        return false;

    return flag == CircleDirectionFlag::CCW ? result < 0 : result > 0;
}

std::vector<WorldPosition> PathGenerator::AStarReconstruction(const std::shared_ptr<ANode> &node)
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
    std::ranges::reverse(result);
    return result;
}

WorldPosition PathGenerator::findNextCirclePos(const WorldPosition &start,
                                               const WorldPosition &center,
                                               const double radius,
                                               const CircleDirectionFlag flag)
{
    // Simple rotation to find the next point
    PathVector center_to_start(center, start);
    assert(std::abs(center_to_start.getMagnitude() - radius) < EP); // just to ensure no bugs from this.

    // NOTE!! opposite from expected because Worlds are inverted
    const double direction_decider = flag == CircleDirectionFlag::CW ? 1 : -1;

    // Find rotation angle: (just math equation of triangle bisect. Not magic numbers)
    const double angle = 2 * std::asin(STEP_SIZE / (2 * radius));

    return center + center_to_start.rotate(angle * direction_decider);
}

WorldPath PathGenerator::makeCircle(const WorldPosition &start,
                                    const WorldPosition &circ_center,
                                    const double circ_radius,
                                    const PathRequest &request,
                                    const PathFlag flag,
                                    const CircleDirectionFlag circle_flag)
{
    std::vector<WorldPosition> result;
    // Assert that the first tile is good
    assert(request.world_grid.IsWalkable(start));
    // Now we should be good to make the circle from here and take whatever strategy to make it.
    result.push_back(start);

    if (flag == PathFlag::Skip)
    {
        WorldPosition next = findNextCirclePos(start, circ_center, circ_radius, circle_flag);

        while (EuclideanDistance(start, next) > STEP_SIZE / 2)
        {
            auto [possible, path] = IsTravelableCircle(result.back(), next, request);
            if (possible)
            {
                // Possible to optimize this. but result isn't always >= 2 size ....
                result.insert(result.end(), path.begin() + 1, path.end());
            }
            next = findNextCirclePos(next, circ_center, circ_radius, circle_flag);
        }
        // add the last point provided that 1. it is a valid tile and 2. that it comes before the start of the circle

        auto [possible, path] = IsTravelableCircle(result.back(), next, request);
        if (possible)
        {
            if (path.size() >= 2)
                result.insert(result.end(), path.begin() + 1, path.end() - 1);
            if (IsPointBefore(next, start, circ_center, circle_flag) && EuclideanDistance(result.back(), start) >
                STEP_SIZE * STEP_CIRCLE_TOLERANCE)
                result.push_back(next);
        }
        else
        {
            // Tile invalid just go to the start - didn't do this originally since it is slightly different in
            // cases that matter
            auto [possible2, path2] = IsTravelableCircle(result.back(), start, request);
            if (possible2)
                result.insert(result.end(), path2.begin(), path2.end() - 1);
        }

        return WorldPath(result);
    }
    if (flag == PathFlag::Expand) // ensures this is the case if other cases are added.
    {
        // this case is strange and needs to be updated IMMEDIATELY once physics changes and more information exists
        // TODO: Update this once physics and more information about positions exists, size of player, etc.
        //TODO: Expand is supposed to hug walls and corners but is impossible without size of player/collision test
        // This case is also really slow but I think it is useful for calculating things and saving them in jsons
        WorldPosition next = findNextCirclePos(start, circ_center, circ_radius);

        auto inner = [&]
        {
            auto [possible, _] = IsTravelableCircle(result.back(), next, request);
            // this should run small code every time
            if (possible)
                result.push_back(next); // We do not extend paths. Just need to know it is possible or not
            else
            {
                const double search_distance = PathVector(result.back(), next).getMagnitude() *
                    CIRCLE_EXPAND_MULTIPLIER;
                auto path = AStarSearch(result.back(), request, SingleGoalHeuristic{next}, search_distance, true);
                if (path.size() > 1)
                    result.insert(result.end(), path.begin() + 1, path.end()); // skip first pos for repeats
            }
            next = findNextCirclePos(next, circ_center, circ_radius);
        };

        // default step unit is coded as 1 for travel directions. this is 1/2
        while (EuclideanDistance(start, next) > STEP_SIZE / 2)
        {
            inner();
        }
        // once within range of ending

        // This case is different from the previous ending connection because in this case we are
        // always technically very close to the starting position so we only worry if it is a missing point
        // to fill the gap
        if (IsPointBefore(next, start, circ_center, circle_flag) && EuclideanDistance(result.back(), start) > 1)
        {
            const size_t size_temp = result.size();
            inner(); // call it one more time with that position
            if (size_temp != result.size())
                return WorldPath(result); // point increased it is close enough and can loop
            // ELSE:
            // Nothing was added. Try to connect to the start point instead:
            auto [ending2_possible, ending2_path] = IsTravelableCircle(result.back(), start, request);
            if (ending2_possible)
                result.insert(result.end(), ending2_path.begin(), ending2_path.end() - 1);
        }

        return WorldPath(result);
    }
    // else empty. Should never happen unless more conditions are added
    std::unreachable();
}

std::optional<CirclePath> PathGenerator::FindCircularPath(const WorldPosition &agent_pos,
                                                          const WorldPosition &circ_center,
                                                          const double circ_radius,
                                                          const PathRequest &request,
                                                          const PathFlag flag,
                                                          const CircleDirectionFlag circle_flag)
{
    /*
     * Goal is to return a list of points that are 1 unit apart for the most part making a complete circle
     * The start of the circle is at the point nearest to the agent given that tile is reachable. Otherwise it is
     * the top of the circle. --> If top is unreachable then it will be the first point that appears that is reachable
     * from the top and going clockwise. If you want to go counterclockwise then the list is just reversed -- that
     * can be done later.
     */

    // Step 1: Find the starting point
    auto circle_dir = PathVector(agent_pos, circ_center);
    circle_dir.normalize().scale(circ_radius * -1);
    auto start_point = circ_center + circle_dir;
    std::vector<WorldPosition> path_to_circle;

    // We need to encapsulate the process of finding the initial path to a point on the circle before generating the circle
    // It is two separate things and shouldn't be combined because then it needs to be separated again.
    // HOWEVER, the processes are similar so there is a little bit of redundancy that is bad to combine into 1 func
    auto [easy_travel_possible, easy_travel_path] = IsTravelableCircle(agent_pos, start_point, request);
    if (easy_travel_possible)
        path_to_circle.insert(path_to_circle.end(), easy_travel_path.begin(), easy_travel_path.end());
    else
    {
        // Try to find start point
        WorldPosition next = findNextCirclePos(start_point, circ_center, circ_radius, circle_flag);
        while (EuclideanDistance(start_point, next) > STEP_SIZE / 2)
        {
            auto [possible, path] = IsTravelableCircle(agent_pos, next, request);
            if (possible)
            {
                // Possible to optimize this. but result isn't always >= 2 size ....
                path_to_circle.insert(path_to_circle.end(), path.begin(), path.end());
                start_point = next;
                break;
            }
            next = findNextCirclePos(next, circ_center, circ_radius, circle_flag);
        }
    }
    if (path_to_circle.empty())
        return {};

    // We should have a valid path now to the start of the circle and a position of the circle.
    // Now we just need to know how to loop around the circle
    const auto circle_path = makeCircle(start_point, circ_center, circ_radius, request, flag, circle_flag);

    // Need to fully write it out here.
    return CirclePath(WorldPath(path_to_circle), circle_path);
}

std::optional<std::vector<WorldPosition> > PathGenerator::MakeRectangleLoop(const WorldPosition &bot_left,
                                                                            const WorldPosition &top_right,
                                                                            const PathRequest &request,
                                                                            const CircleDirectionFlag flag)
{
    // NOTE: does this return cell int positions or doubles? -- doubles if given doubles
    assert(bot_left.CellX() <= top_right.CellX());
    assert(bot_left.CellY() >= top_right.CellY());
    std::vector<WorldPosition> result;
    WorldPosition cur = bot_left;
    const size_t up_steps = bot_left.CellY() - top_right.CellY();
    const size_t right_steps = top_right.CellX() - bot_left.CellX();
    const std::array directions = {
        PathVector(0, -STEP_SIZE), PathVector(STEP_SIZE, 0),
        PathVector(0, STEP_SIZE), PathVector(-STEP_SIZE, 0)
    };
    const std::array steps = {up_steps, right_steps, up_steps, right_steps};
    for (size_t i = 0; i < directions.size(); i++)
    {
        for (size_t j = 0; j < steps.at(i); j++)
        {
            result.push_back(cur);
            cur = cur + directions.at(i);
            if (request.avoid_tiles.contains(round(cur)))
                return {};
        }
    }
    if (flag == CircleDirectionFlag::CCW)
        std::ranges::reverse(result);
    return result;
}

std::optional<CirclePath> PathGenerator::FindRectangularLoopPath(const WorldPosition &agent_pos,
                                                                 const WorldPosition &bot_left,
                                                                 const WorldPosition &top_right,
                                                                 const PathRequest &request,
                                                                 const CircleDirectionFlag flag)
{
    // finds rectangular loop. Quite simple pretty much just creates it if it can.
    // 1. First find the path from the agent bosition to the loop
    // TODO MULTIGOAL H. You need to make map of INTS Cell positions for comparison in calcs.

    std::unordered_set<WorldPosition> goals;
    const auto goals_vec = MakeRectangleLoop(bot_left, top_right, request, flag);
    if (!goals_vec)
        return {};
    const auto &loop_vec = goals_vec.value();
    for (const auto &each : loop_vec)
    {
        goals.insert({each.CellX(), each.CellY()});
    }

    auto start_path = AStarSearch(agent_pos, request, MultiGoalHeuristic{goals});
    if (start_path.empty())
        return {};
    assert(goals.contains(round(start_path.back())));

    const auto loop_start = start_path.back();
    std::vector<WorldPosition> loop_path;
    size_t start_index = 0;
    for (size_t i = 0; i < loop_vec.size(); i++)
    {
        if (round(loop_start) == round(loop_vec.at(i)))
        {
            start_index = i;
            break;
        }
    }
    // The original start_path worked with finding cell positions, not actual positions. This can affect
    // The creation of the loop vs intended.
    if ((loop_start - loop_vec.at(start_index)).getMagnitude() > EP)
        start_path.push_back(loop_vec.at(start_index));
    for (size_t i = 0; i < loop_vec.size(); i++)
    {
        const size_t index = (start_index + i) % loop_vec.size();
        loop_path.push_back(loop_vec.at(index));
    }

    return CirclePath(WorldPath(start_path), WorldPath(loop_path));
}
std::optional<WorldPath> PathGenerator::FindShortestPath(const WorldPosition &start,
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
    auto result = AStarSearch(start, request, SingleGoalHeuristic{end});
    if (result.empty())
        return {};
    return WorldPath(result);
}
std::optional<WorldPath> PathGenerator::FindManhattanPath(const WorldPosition &start,
                                                          const WorldPosition &end,
                                                          const PathRequest &request,
                                                          const CircleDirectionFlag flag)
{
    if (!IsTravelable(start, {0, 0}, request)) // Check start position. A little verbose but simple
        return {};

    std::vector<WorldPosition> result;
    // Quad I, III move up and down first for CW        II, IV horizontal for CW
    if (!start.IsValid() || !end.IsValid())
        return {};
    const int startx = static_cast<int>(start.CellX());
    const int starty = static_cast<int>(start.CellY());
    const int endx = static_cast<int>(end.CellX());
    const int endy = static_cast<int>(end.CellY());
    int const direction_determiner = (endx - startx) * (endy - starty);
    // if (direction_determiner == 0)

    bool const top_down = (flag != CircleDirectionFlag::CW || direction_determiner <= 0) &&
        (flag == CircleDirectionFlag::CW || direction_determiner >= 0);

    auto fup_down = [&](const WorldPosition &start_pos)
    {
        int const direction_sign = endy - starty >= 0 ? 1 : -1;
        auto const direction = PathVector(0, 1).mult(0, direction_sign);
        size_t const steps = std::abs(endy - starty);
        WorldPosition cur = start_pos;
        for (size_t i = 0; i < steps; i++)
        {
            if (!IsTravelable(cur, direction, request))
                return false; // Cannot make a simple path;
            cur = cur + direction;
            result.push_back(cur);
        }
        return true;
    };
    auto fright_left = [&](const WorldPosition &start_pos)
    {
        int const direction_sign = endx - startx >= 0 ? 1 : -1;
        auto const direction = PathVector(1, 0).mult(direction_sign, 0);
        size_t const steps = std::abs(endx - startx);
        WorldPosition cur = start_pos;
        for (size_t i = 0; i < steps; i++)
        {
            if (!IsTravelable(cur, direction, request))
                return false;
            cur = cur + direction;
            result.push_back(cur);
        }
        return true;
    };

    result.push_back(start);
    bool r1;
    bool r2;
    if (top_down)
    {
        // to top then right
        r1 = fup_down(start);
        r2 = fright_left(result.back());
    }
    else
    {
        // do right then top
        r1 = fright_left(start);
        r2 = fup_down(result.back());
    }
    if (!r1 || !r2)
        return {};
    return WorldPath(result);

    // Test Case: check 2D plane given center is the start point and check every point
}
}
