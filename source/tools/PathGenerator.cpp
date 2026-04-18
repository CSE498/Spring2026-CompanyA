/**
 * @file PathGenerator.cpp
 * @author Logan Rimarcik
 */

#include "PathGenerator.hpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <numbers>
#include <unordered_map>
#include <unordered_set>
#include "../core/WorldGrid.hpp"
#include "WorldPath.hpp"

namespace cse498 {

double PathGenerator::EuclideanDistance(const WorldPosition& p1, const WorldPosition& p2) {
    const double x_x = p1.X() - p2.X();
    const double y_y = p1.Y() - p2.Y();
    return std::sqrt(x_x * x_x + y_y * y_y); // can't be constexpr but c++26 is nearby
}


double PathGenerator::ManhattanDistance(const WorldPosition& p1, const WorldPosition& p2) {
    return std::abs(p1.X() - p2.X()) + std::abs(p1.Y() - p2.Y());
}


bool PathGenerator::IsTravelable(const WorldPosition& from, const PathVector& dir, const PathRequest& request) {
    auto nextPos = from + dir;
    if (!request.mWorldGrid.IsWalkable(nextPos) || request.mAvoidTiles.contains(Round(nextPos)))
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
    const WorldPosition test1 = from + dir.Mult({1, 0});
    const WorldPosition test2 = from + dir.Mult({0, 1});
    return request.mWorldGrid.IsWalkable(test1) && request.mWorldGrid.IsWalkable(test2);
}

PathGenerator::CircleTravel PathGenerator::IsTravelableCircle(const WorldPosition& from, const WorldPosition& to,
                                                              const PathRequest& request) {
    // find the direction of the 'to'
    const auto vec = PathVector(from, to);
    if (vec.GetMagnitude() > 1 + EP) // can't be constexpr
    {
        // Some point was skipped. We no longer know if we can travel there directly: Find weird path there instead
        auto pth = AStarSearch(from, request, SingleGoalHeuristic{to}, vec.GetMagnitude() * 2, false);
        if (!pth.empty()) {
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
bool PathGenerator::IsPointBefore(const WorldPosition& testPt, const WorldPosition& relativePt,
                                  const WorldPosition& center, const CircleDirectionFlag flag) {
    // Again clockwise = Counterclockwise and vise versa. Everything is opposite and it hurts my head
    // Assume both points are on the circle
    const PathVector centerToTest(testPt, center);
    const PathVector centerToRelative(relativePt, center);
    assert(std::abs(centerToRelative.GetMagnitude() - centerToTest.GetMagnitude()) < EP);

    const PathVector test(testPt.X() - center.X(), testPt.Y() - center.Y());
    const PathVector relative(relativePt.X() - center.X(), relativePt.Y() - center.Y());

    // This determines if a point is before another point by comparing the angles they make.
    // The weird if statements are because of how the angle is returned from getAngle -- it returns in domain [-pi, pi]
    double testAngle = test.GetAngle(); // Not constexpr
    double relAngle = relative.GetAngle();
    if (flag == CircleDirectionFlag::CW && relAngle < 0 && testAngle > 0)
        relAngle += 2 * std::numbers::pi;
    if (flag == CircleDirectionFlag::CCW && relAngle > 0 && testAngle < 0)
        testAngle += 2 * std::numbers::pi;
    const double result = relAngle - testAngle;
    if (std::abs(result) > std::numbers::pi)
        return false;

    return flag == CircleDirectionFlag::CCW ? result < 0 : result > 0;
}


std::vector<WorldPosition> PathGenerator::AStarReconstruction(const ANode* end) {
    std::vector<WorldPosition> result;
    // Reconstruct the path and return.
    result.push_back(end->mPos);
    auto prev = end->mPrev;
    while (prev) {
        result.push_back(prev->mPos);
        prev = prev->mPrev;
    }
    std::ranges::reverse(result);
    return result;
}

WorldPosition PathGenerator::FindNextCirclePos(const WorldPosition& start, const WorldPosition& center,
                                               const double radius, const CircleDirectionFlag flag) {
    // Simple rotation to find the next point
    PathVector centerToStart(center, start);
    assert(std::abs(centerToStart.GetMagnitude() - radius) < EP); // just to ensure no bugs from this.

    // NOTE!! opposite from expected because Worlds are inverted
    const double directionDecider = flag == CircleDirectionFlag::CW ? 1 : -1;

    // Find rotation angle: (just math equation of triangle bisect. Not magic numbers)
    const double angle = 2 * std::asin(STEP_SIZE / (2 * radius)); // can't be constexpr

    return center + centerToStart.Rotate(angle * directionDecider);
}

WorldPath PathGenerator::MakeCircle(const WorldPosition& start, const WorldPosition& circCenter, const double cRadius,
                                    const PathRequest& request, const PathFlag flag,
                                    const CircleDirectionFlag circleFlag) {
    std::vector<WorldPosition> result;
    // Assert that the first tile is good
    assert(request.mWorldGrid.IsWalkable(start));
    // Now we should be good to make the circle from here and take whatever strategy to make it.
    result.push_back(start);

    if (flag == PathFlag::Skip) {
        WorldPosition next = FindNextCirclePos(start, circCenter, cRadius, circleFlag);

        while (EuclideanDistance(start, next) > STEP_SIZE / 2) {
            auto [possible, path] = IsTravelableCircle(result.back(), next, request);
            if (possible) {
                // Possible to optimize this. but result isn't always >= 2 size ....
                result.insert(result.end(), path.begin() + 1, path.end());
            }
            next = FindNextCirclePos(next, circCenter, cRadius, circleFlag);
        }
        // add the last point provided that 1. it is a valid tile and 2. that it comes before the start of the circle

        auto [possible, path] = IsTravelableCircle(result.back(), next, request);
        if (possible) {
            if (path.size() >= 2)
                result.insert(result.end(), path.begin() + 1, path.end() - 1);
            if (IsPointBefore(next, start, circCenter, circleFlag) &&
                EuclideanDistance(result.back(), start) > STEP_SIZE * STEP_CIRCLE_TOLERANCE)
                result.push_back(next);
        } else {
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
        // TODO: Expand is supposed to hug walls and corners but is impossible without size of player/collision test
        // This case is also really slow but I think it is useful for calculating things and saving them in jsons
        WorldPosition next = FindNextCirclePos(start, circCenter, cRadius);

        auto inner = [&] {
            auto [possible, _] = IsTravelableCircle(result.back(), next, request);
            // this should run small code every time
            if (possible)
                result.push_back(next); // We do not extend paths. Just need to know it is possible or not
            else {
                const double searchDistance = PathVector(result.back(), next).GetMagnitude() * CIRCLE_EXPAND_MULTIPLIER;
                auto path = AStarSearch(result.back(), request, SingleGoalHeuristic{next}, searchDistance, true);
                if (path.size() > 1)
                    result.insert(result.end(), path.begin() + 1, path.end()); // skip first pos for repeats
            }
            next = FindNextCirclePos(next, circCenter, cRadius);
        };

        // default step unit is coded as 1 for travel directions. this is 1/2
        while (EuclideanDistance(start, next) > STEP_SIZE / 2) {
            inner();
        }
        // once within range of ending

        // This case is different from the previous ending connection because in this case we are
        // always technically very close to the starting position so we only worry if it is a missing point
        // to fill the gap
        if (IsPointBefore(next, start, circCenter, circleFlag) && EuclideanDistance(result.back(), start) > 1) {
            const size_t sizeTemp = result.size();
            inner(); // call it one more time with that position
            if (sizeTemp != result.size())
                return WorldPath(result); // point increased. it is close enough and can loop
            // ELSE:
            // Nothing was added. Try to connect to the start point instead:
            auto [isEnding, endPath] = IsTravelableCircle(result.back(), start, request);
            if (isEnding)
                result.insert(result.end(), endPath.begin(), endPath.end() - 1);
        }

        return WorldPath(result);
    }
    // else empty. Should never happen unless more conditions are added
    std::unreachable();
}

std::optional<CirclePath> PathGenerator::FindCircularPath(const WorldPosition& agentPos,
                                                          const WorldPosition& circleCenter, const double cRadius,
                                                          const PathRequest& request, const PathFlag flag,
                                                          const CircleDirectionFlag circleFlag) {
    /*
     * Goal is to return a list of points that are 1 unit apart for the most part making a complete circle
     * The start of the circle is at the point nearest to the agent given that tile is reachable. Otherwise it is
     * the top of the circle. --> If top is unreachable then it will be the first point that appears that is reachable
     * from the top and going clockwise. If you want to go counterclockwise then the list is just reversed -- that
     * can be done later.
     */

    // Step 1: Find the starting point
    auto circleDir = PathVector(agentPos, circleCenter);
    circleDir.Normalize().Scale(cRadius * -1);
    auto startPoint = circleCenter + circleDir;
    std::vector<WorldPosition> pathToCircle;

    // We need to encapsulate the process of finding the initial path to a point on the circle before generating the
    // circle It is two separate things and shouldn't be combined because then it needs to be separated again. HOWEVER,
    // the processes are similar so there is a little bit of redundancy that is bad to combine into 1 func
    auto [travelPossible, travelPath] = IsTravelableCircle(agentPos, startPoint, request);
    if (travelPossible)
        pathToCircle.insert(pathToCircle.end(), travelPath.begin(), travelPath.end());
    else {
        // Try to find start point
        WorldPosition next = FindNextCirclePos(startPoint, circleCenter, cRadius, circleFlag);
        while (EuclideanDistance(startPoint, next) > STEP_SIZE / 2) {
            auto [possible, path] = IsTravelableCircle(agentPos, next, request);
            if (possible) {
                // Possible to optimize this. but result isn't always >= 2 size ....
                pathToCircle.insert(pathToCircle.end(), path.begin(), path.end());
                startPoint = next;
                break;
            }
            next = FindNextCirclePos(next, circleCenter, cRadius, circleFlag);
        }
    }
    if (pathToCircle.empty())
        return {};

    // We should have a valid path now to the start of the circle and a position of the circle.
    // Now we just need to know how to loop around the circle
    const auto circlePath = MakeCircle(startPoint, circleCenter, cRadius, request, flag, circleFlag);

    // Need to fully write it out here.
    return CirclePath(WorldPath(pathToCircle), circlePath);
}

std::optional<std::vector<WorldPosition>> PathGenerator::MakeRectangleLoop(const WorldPosition& bottomLeft,
                                                                           const WorldPosition& topRight,
                                                                           const PathRequest& request,
                                                                           const CircleDirectionFlag flag) {
    // NOTE: does this return cell int positions or doubles? -- doubles if given doubles
    assert(bottomLeft.CellX() <= topRight.CellX());
    assert(bottomLeft.CellY() >= topRight.CellY());
    std::vector<WorldPosition> result;
    WorldPosition cur = bottomLeft;
    const size_t upStepCount = bottomLeft.CellY() - topRight.CellY(); // Not constexpr but could be.
    const size_t rightStepCount = topRight.CellX() - bottomLeft.CellX();
    const std::array directions = {PathVector(0, -STEP_SIZE), PathVector(STEP_SIZE, 0), PathVector(0, STEP_SIZE),
                                   PathVector(-STEP_SIZE, 0)};
    const std::array steps = {upStepCount, rightStepCount, upStepCount, rightStepCount};
    for (size_t i = 0; i < directions.size(); i++) {
        for (size_t j = 0; j < steps.at(i); j++) {
            result.push_back(cur);
            cur = cur + directions.at(i);
            if (request.mAvoidTiles.contains(Round(cur)))
                return {};
        }
    }
    if (flag == CircleDirectionFlag::CCW)
        std::ranges::reverse(result);
    return result;
}

std::optional<CirclePath> PathGenerator::FindRectangularLoopPath(const WorldPosition& agentPos,
                                                                 const WorldPosition& bottomLeft,
                                                                 const WorldPosition& topRight,
                                                                 const PathRequest& request,
                                                                 const CircleDirectionFlag flag) {
    // finds rectangular loop. Quite simple pretty much just creates it if it can.
    // 1. First find the path from the agent bosition to the loop
    // TODO MULTIGOAL H. You need to make map of INTS Cell positions for comparison in calcs.

    std::unordered_set<WorldPosition> goals;
    auto loop = MakeRectangleLoop(bottomLeft, topRight, request, flag);
    if (!loop)
        return {};
    auto& loopVector = loop.value(); // just reference this value and modify it. We own it in loop

    // Forcing std::ranges ..
    // Necessary to make goal set for the multiGoalHeuristic in A*
    std::ranges::for_each(loopVector, [&goals](const auto& tile) { goals.insert({tile.CellX(), tile.CellY()}); });

    auto startPath = AStarSearch(agentPos, request, MultiGoalHeuristic{goals});
    if (startPath.empty())
        return {};
    const auto loopStart = startPath.back(); // ensure it exists in the goals.
    assert(goals.contains(Round(loopStart)));

    // Get the loop in the proper order.
    auto itr =
            std::ranges::find_if(loopVector, [loopStart](const auto& tile) { return Round(loopStart) == Round(tile); });
    if (itr == loopVector.end()) // should always be able to find it by definition
        return {};
    if ((loopStart - *itr).GetMagnitude() > EP) // so little deflections aren't made in the loop also no repeats
        startPath.push_back(*itr);

    // Move the start of the vector over!
    std::ranges::rotate(loopVector, itr);

    return CirclePath(WorldPath(startPath), WorldPath(loopVector));
}
std::optional<WorldPath> PathGenerator::FindShortestPath(const WorldPosition& start, const WorldPosition& end,
                                                         const PathRequest& request) {
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
std::optional<WorldPath> PathGenerator::FindManhattanPath(const WorldPosition& start, const WorldPosition& end,
                                                          const PathRequest& request, const CircleDirectionFlag flag) {
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
    int const directionDeterminer = (endx - startx) * (endy - starty);
    // if (direction_determiner == 0)

    bool const topDown = (flag != CircleDirectionFlag::CW || directionDeterminer <= 0) &&
                         (flag == CircleDirectionFlag::CW || directionDeterminer >= 0);

    auto upDown = [&](const WorldPosition& startPos) {
        int const directionSign = endy - starty >= 0 ? 1 : -1;
        auto const direction = PathVector(0, 1).Mult(0, directionSign);
        size_t const steps = std::abs(endy - starty);
        WorldPosition cur = startPos;
        for (size_t i = 0; i < steps; i++) {
            if (!IsTravelable(cur, direction, request))
                return false; // Cannot make a simple path;
            cur = cur + direction;
            result.push_back(cur);
        }
        return true;
    };
    auto rightLeft = [&](const WorldPosition& startPos) {
        int const directionSign = endx - startx >= 0 ? 1 : -1;
        auto const direction = PathVector(1, 0).Mult(directionSign, 0);
        size_t const steps = std::abs(endx - startx);
        WorldPosition cur = startPos;
        for (size_t i = 0; i < steps; i++) {
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
    if (topDown) {
        // to top then right
        r1 = upDown(start);
        r2 = rightLeft(result.back());
    } else {
        // do right then top
        r1 = rightLeft(start);
        r2 = upDown(result.back());
    }
    if (!r1 || !r2)
        return {};
    return WorldPath(result);
}


bool PathGenerator::IsPathClear(const WorldPosition& start, const PathVector& pathDir, const PathRequest& request) {

    /*
     * Points are checked at (x,y) being the TOP LEFT position (straight line between then if
     * the line enters grid then that grid is checked.
     * If the line is a tie like (1,1) --> (2,2) then it goes Y DIRECTION first (in this case down to (2,1)
     * then over to (2,2). It is a tie through a point and can't do diagonal steps like that though could.
     * --> Just modify inner loop to check equality and increment both.
     */

    // I'm just using this lambda as an inner function loop instead of a function elsewhere
    auto inner = [start](const WorldPosition& rStart, const WorldPosition& endTile, const PathVector& offset,
        const PathVector& path, const PathRequest &request)
    {

        /*
         * I shift the start of the ray by offset and go in the same direction with the
         * same magnitude so the end point is endTile + offset, but it will always be
         * captured by tileX and tileY so we never use that term.
         */

        auto startShifted = start + offset;
        auto tileX = rStart.X();
        auto tileY = rStart.Y();


        int stepX = (path.X() > 0) ? 1 : -1;
        int stepY = (path.Y() > 0) ? 1 : -1;

        // this bit is adapted from chatgpt telling me about "2D DDA grid raycast" but is simple
        // chatgpt just sped it up. No chat needed. Just look up ^^^.

        // in short:
        // essentially value of 't' between two points like x = 3 and x = 4 -- how much needed Solves:
        // 1. startX + V_X * t = 3
        // 2. startX + V_X * t = 4
        // solve for the difference in 't' = delta T. (eq2 - eq1)
        double tDeltaX = (path.X() != 0) ? std::abs(1 / path.X()) : INFINITY;
        double tDeltaY = (path.Y() != 0) ? std::abs(1 / path.Y()) : INFINITY;

        // This part is really simple we are just solving for 't' in:
        //  startX + V.X() * t = floor(startX) + 1
        // from start position how much of time step to reach the next barrier (depending on direction)
        double nextBoundaryX = (stepX > 0) ? (tileX + 1) : tileX;
        double nextBoundaryY = (stepY > 0) ? (tileY + 1) : tileY;
        double tMaxX = (path.X() != 0) ? (nextBoundaryX - startShifted.X()) / path.X() : INFINITY;
        double tMaxY = (path.Y() != 0) ? (nextBoundaryY - startShifted.Y()) / path.Y() : INFINITY;

        while ((endTile - WorldPosition(tileX, tileY)).GetMagnitude() > EP)
        {
            if (std::abs(tMaxX - tMaxY) < EP)
            {
                // Check both corner tiles
                if (!request.mWorldGrid.IsWalkable({tileX + stepX, tileY}))
                    return false;
                if (!request.mWorldGrid.IsWalkable({tileX, tileY + stepY}))
                    return false;
                // now increment
                tileX += stepX;
                tileY += stepY;
                tMaxX += tDeltaX;
                tMaxY += tDeltaY;
                // We basically just combined two operations into one checking both sides because of tie
            }
            if (tMaxX < tMaxY)
            {
                tileX += stepX;
                tMaxX += tDeltaX; // increment the 't' value by 1 because 1 more step is now needed to reach next tiles
            }
            else
            {
                tileY += stepY;
                tMaxY += tDeltaY;
            }
            if (!request.mWorldGrid.IsWalkable({tileX, tileY}))
                return false;
        }
        return true;

    };

    // Expensive but not that expensive. This gives a more comprehensive check
    // so that edges of blocks are ignored in hit testing.

    // We make an algorithm to go over all tiles that are entered and check if those tiles are walls
    WorldPosition startRounded = Round(start);
    WorldPosition endTile = Round(start + pathDir);

    PathVector path = pathDir;
    path.Normalize();

    // Just one test from the CENTER OF THE TILE
    return inner(startRounded, endTile, {0.5,0.5}, path, request);


}

WorldPath PathGenerator::FindPointAway(const WorldPosition& start, const WorldPosition& center,
                                       const PathRequest& request, double radius) {
    if (!start.IsValid() || !center.IsValid() || radius < 0.0)
        return {};

    // If already at the proper distance then done
    const double startDist = EuclideanDistance(start, center);
    if (startDist + EP >= radius)
        return WorldPath();

    PathVector dir = start - center;
    dir.Normalize(); // Just to scale it to reasonable size so no issues with ~ 0 occur.
    auto path = FindFurtherestPoint(start, center, dir, request, radius);
    return WorldPath(path);
}

std::vector<WorldPosition> PathGenerator::FindFurtherestPoint(const WorldPosition& start, const WorldPosition& center,
                                                              const PathVector& direction, const PathRequest& request,
                                                              double maxRange) {
    // If the direction is small we say that 0 direction was provided {0,0} which is invalid
    if (!request.mWorldGrid.IsWalkable(start) || direction.GetMagnitude() < EP)
        return {};

    // Possible directions that can be taken
    std::array<PathVector, 3> dirs = std::to_array<PathVector>({{0, 0}, {0, 0}, {0, 0}});
    int dirCount = 0;

    // It is unintended if direction is small to take those directions
    // ensure these are int directions but saved as doubles because of "narrowing" conversion warnings int --> double?
    double dx = (std::abs(direction.X()) < EP) ? 0 : static_cast<int>(direction.X() / std::abs(direction.X()));
    double dy = (std::abs(direction.Y()) < EP) ? 0 : static_cast<int>(direction.Y() / std::abs(direction.Y()));
    constexpr int STEP = 1; // Defined by the two lines above

    if (dx && dy) // likely case so separated and done first
    {
        dirs[0] = {dx, 0};
        dirs[1] = {0, dy};
        dirCount = 2;
    } else {
        dirCount = 3;
        if (dx) {
            dirs[0] = {dx, 0};
            dirs[1] = {0, STEP}; // this is hard coded 1 since dx,dy calculated to be hard coded 1
            dirs[2] = {0, -STEP};
        } else {
            dirs[0] = {0, dy};
            dirs[1] = {-STEP, 0};
            dirs[2] = {STEP, 0};
        }
    }

    const bool canMove0 = IsTravelable(start, dirs[0], request);
    const bool canMove1 = IsTravelable(start, dirs[1], request);
    const bool canMove2 = (dirCount == 3) ? IsTravelable(start, dirs[2], request) : false;

    if (!canMove0 && !canMove1 && !canMove2)
        return {};
    double startg = EuclideanDistance(start, center);


    // ---- DFS setup
    std::vector<std::unique_ptr<ANode>> storage;
    std::vector<ANode*> stack;
    std::unordered_set<WorldPosition> visited;

    // start node
    storage.push_back(std::make_unique<ANode>(start, startg, 0.0, nullptr));
    stack.push_back(storage.back().get());
    visited.insert(start);

    // track best result if we never reach maxRange
    ANode* bestNode = storage.back().get();

    while (!stack.empty()) {
        ANode* node = stack.back();
        stack.pop_back();

        // update best node (furthest by path cost)
        if (node->mg > bestNode->mg)
            bestNode = node;

        // goal condition: reached radius → return immediately
        if (node->mg >= maxRange - EP) // this limits depth
        {
            return AStarReconstruction(node);
        }

        // Otherwise continue exploring
        for (int i = dirCount - 1; i >= 0; --i) // reverse so dirs[0] explored first
        {
            const auto& dir = dirs[i];
            WorldPosition neighbor = node->mPos + dir;

            // if the neighbor is not a valid walking tile then skip it
            if (!IsTravelable(node->mPos, dir, request))
                continue;

            // I want an actual circle and not a circle dependent on manhattan distance
            double g = EuclideanDistance(neighbor, center);

            if (visited.contains(neighbor))
                continue;

            visited.insert(neighbor);

            // Order of arguments is Node, g, f, prev node (f unused here)
            storage.push_back(std::make_unique<ANode>(neighbor, g, 0.0, node));
            stack.push_back(storage.back().get());
        }
    }

    // If we never hit maxRange, return the furthest we found
    return AStarReconstruction(bestNode);
}


} // namespace cse498
