#include "SmartEnemyAgent.hpp"

#include <array>
#include <limits>
#include <queue>
#include <unordered_map>
#include <unordered_set>

#include "../../Worlds/DemoG1/AIWorld.hpp"

namespace cse498 {

namespace {
/// The size of mRecentPositions, works like short term memory
constexpr size_t kRecentMemory = 10;
} // namespace

/**
 * Initializes the smart enemy agent. Sets the display symbol for the agent to 'S'.
 *
 * @return true if initialization succeeds.
 */
bool SmartEnemyAgent::Initialize() {
    SetSymbol('S');
    return true;
}

/**
 * @brief Determines whether a world position can be traversed. A position is considered walkable if it is within the
 * grid bounds and does not contain a wall tile ('#').
 *
 * @param grid The world grid being queried.
 * @param pos The position to test.
 * @return true if the position is valid and not blocked by a wall.
 */
bool SmartEnemyAgent::IsWalkable(const WorldGrid& grid, WorldPosition pos) const {
    if (!grid.IsValid(pos))
        return false;
    const char tile = grid.GetCellTypeSymbol(grid[pos]);
    return tile != '#';
}

    /**
     * @brief Picks a target position for pursuit.
     *
     * @details Resolution order:
     *   1. @ref WorldBase::GetTrackedPlayerPosition() if the world overrides it
     *      (e.g. @c DungeonWorld returns its live keyboard-player tile). This lets
     *      the AI chase players that are not modeled as @ref AgentBase instances.
     *   2. Otherwise, scans @ref WorldBase::GetAgents() and selects the nearest
     *      non-self agent by Manhattan distance. Used by @c AIWorld / @c group01_demo
     *      where the "player" is a registered agent.
     *
     * @return The chosen target position, or @c std::nullopt if the agent has no
     *         valid location or no candidate targets exist.
     */
    std::optional<WorldPosition> SmartEnemyAgent::GetTargetPlayerPosition() const {
        if (!GetLocation().IsPosition())
            return std::nullopt;

        // Prefer the world's tracked keyboard-player position when available
        // (DungeonWorld pushes {mPlayerX, mPlayerY} here each frame). Falls back
        // to the agent-iteration scan so AIWorld / group01_demo behaves identically.
        if (auto tracked = world.GetTrackedPlayerPosition(); tracked.has_value()) {
            return tracked;
        }

        const WorldPosition my_pos = GetLocation().AsWorldPosition();
        int best_dist = std::numeric_limits<int>::max();
        std::optional<WorldPosition> best_target;

        for (size_t i = 0; i < world.GetNumAgents(); ++i) {
            const AgentBase &agent = world.GetAgent(i);

        if (agent.GetID() == GetID())
            continue;
        if (!agent.GetLocation().IsPosition())
            continue;

        const WorldPosition pos = agent.GetLocation().AsWorldPosition();
        const int dist = static_cast<int>(std::abs(static_cast<int>(my_pos.CellX()) - static_cast<int>(pos.CellX())) +
                                          std::abs(static_cast<int>(my_pos.CellY()) - static_cast<int>(pos.CellY())));

        if (dist < best_dist) {
            best_dist = dist;
            best_target = pos;
        }
    }

    return best_target;
}

/**
 * @brief Determines whether an adjacent target can be attacked immediately.
 * If the current target is in one of the four neighboring cells, this returns the corresponding attack action ID.
 *
 * @return The attack action ID if the target is adjacent.
 */
std::optional<size_t> SmartEnemyAgent::AttackActionForAdjacentPlayer() const {
    const auto target = GetTargetPlayerPosition();
    if (!target.has_value() || !GetLocation().IsPosition())
        return std::nullopt;

    const WorldPosition cur = GetLocation().AsWorldPosition();

    if (cur.Up() == *target)
        return GetActionID("attack_up");
    if (cur.Down() == *target)
        return GetActionID("attack_down");
    if (cur.Left() == *target)
        return GetActionID("attack_left");
    if (cur.Right() == *target)
        return GetActionID("attack_right");

    return std::nullopt;
}

/**
 * @brief Checks whether two positions have a clear horizontal or vertical line of sight.
 * Line of sight is only considered valid when the positions share the same row or column and all cells between them are
 * walkable.
 *
 * @param grid The world grid used for obstruction checks.
 * @param from The starting position.
 * @param to The destination position.
 * @return true if no walls block a straight-line path between the positions.
 */
bool SmartEnemyAgent::HasLineOfSight(const WorldGrid& grid, WorldPosition from, WorldPosition to) const {
    if (from.CellX() == to.CellX()) {
        const size_t x = from.CellX();
        const size_t y1 = std::min(from.CellY(), to.CellY());
        const size_t y2 = std::max(from.CellY(), to.CellY());

        for (size_t y = y1 + 1; y < y2; ++y) {
            if (!IsWalkable(grid, WorldPosition{x, y}))
                return false;
        }
        return true;
    }

    if (from.CellY() == to.CellY()) {
        const size_t y = from.CellY();
        const size_t x1 = std::min(from.CellX(), to.CellX());
        const size_t x2 = std::max(from.CellX(), to.CellX());

        for (size_t x = x1 + 1; x < x2; ++x) {
            if (!IsWalkable(grid, WorldPosition{x, y}))
                return false;
        }
        return true;
    }

    return false;
}

/**
 * @brief Chooses a direct pursuit move when the target is visible in a straight line.
 *
 * @param grid The world grid used to test visibility.
 * @param target The target position to pursue.
 * @return The movement action ID for pursuit.
 */
std::optional<size_t> SmartEnemyAgent::LineOfSightPursuitMove(const WorldGrid& grid, WorldPosition target) const {
    if (!GetLocation().IsPosition())
        return std::nullopt;

    const WorldPosition cur = GetLocation().AsWorldPosition();

    if (!HasLineOfSight(grid, cur, target))
        return std::nullopt;

    if (cur.CellX() == target.CellX()) {
        if (target.CellY() < cur.CellY())
            return GetActionID("up");
        if (target.CellY() > cur.CellY())
            return GetActionID("down");
    }

    if (cur.CellY() == target.CellY()) {
        if (target.CellX() < cur.CellX())
            return GetActionID("left");
        if (target.CellX() > cur.CellX())
            return GetActionID("right");
    }

    return std::nullopt;
}

    /**
     * @brief Computes the next movement step toward a target using breadth-first search.
     *
     * @details The search avoids walls and (when available) other occupied agent
     *          positions, except when the occupied position is the target itself.
     *
     * @note Agent-occupancy blocking is only checked when the containing world is an
     *       @ref AIWorld; other worlds (e.g. @c DungeonWorld) do not expose
     *       @c IsAgentAtPosition, so the cast falls through to @c nullptr and the
     *       agent-blocking branch is skipped. This keeps the algorithm correct in
     *       both environments without requiring a common interface.
     *
     * @param grid The world grid used for pathfinding.
     * @param target The destination position.
     * @return The action ID for the first step along the shortest path, or
     *         @c std::nullopt if no path exists / the agent is unplaced.
     */
    std::optional<size_t> SmartEnemyAgent::NextMoveToward(const WorldGrid &grid, WorldPosition target) const {
        if (!GetLocation().IsPosition())
            return std::nullopt;

        // AIWorld exposes agent-occupancy; DungeonWorld does not, so the cast may fail.
        // When it fails we simply skip the agent-blocking check (no blocking required).
        const auto *ai_world = dynamic_cast<const AIWorld *>(&world);

        const WorldPosition start = GetLocation().AsWorldPosition();
        if (start == target)
            return std::nullopt;

    std::queue<WorldPosition> frontier;
    std::unordered_map<WorldPosition, WorldPosition> parent;
    std::unordered_set<WorldPosition> visited;

    frontier.push(start);
    visited.insert(start);

    while (!frontier.empty()) {
        const WorldPosition pos = frontier.front();
        frontier.pop();

        if (pos == target)
            break;

        const std::array<WorldPosition, 4> neighbors{pos.Up(), pos.Down(), pos.Left(), pos.Right()};

        for (const auto& n: neighbors) {
            if (visited.contains(n))
                continue;
            if (!IsWalkable(grid, n))
                continue;

                // Don't path through other agents unless n is the target tile. (AIWorld only.)
                if (ai_world && ai_world->IsAgentAtPosition(n, GetID()) && !(n == target))
                    continue;

            visited.insert(n);
            parent[n] = pos;
            frontier.push(n);
        }
    }

    if (!visited.contains(target))
        return std::nullopt;

    WorldPosition step = target;
    while (parent.contains(step) && !(parent[step] == start)) {
        step = parent[step];
    }

    const long dx = static_cast<long>(step.CellX()) - static_cast<long>(start.CellX());
    const long dy = static_cast<long>(step.CellY()) - static_cast<long>(start.CellY());

    if (dx == 1 && dy == 0)
        return GetActionID("right");
    if (dx == -1 && dy == 0)
        return GetActionID("left");
    if (dx == 0 && dy == 1)
        return GetActionID("down");
    if (dx == 0 && dy == -1)
        return GetActionID("up");

    return std::nullopt;
}

    /**
     * @brief Chooses a move toward the best reachable tile adjacent to the target.
     * This is used when the target cannot be attacked immediately and is not in
     * direct line of sight. The function evaluates the four attack-adjacent tiles
     * around the target and selects the one with the shortest reachable path.
     *
     * @note Same cross-world behavior as @ref NextMoveToward: agent-occupancy
     *       checks run only when the world is actually an @ref AIWorld. In other
     *       worlds the @c dynamic_cast returns @c nullptr and those checks are
     *       skipped — the path is still computed correctly from wall data alone.
     *
     * @param grid The world grid used for reachability and pathfinding.
     * @param target The target position to surround.
     * @return The action ID for moving toward the best attack lane, or
     *         @c std::nullopt if the agent has no valid location or no lane is
     *         reachable.
     */
    std::optional<size_t> SmartEnemyAgent::ShortestAttackLaneMove(const WorldGrid &grid, WorldPosition target) const {
        if (!GetLocation().IsPosition())
            return std::nullopt;

        // AIWorld exposes agent-occupancy; DungeonWorld does not. Fall back to no agent check.
        const auto *ai_world = dynamic_cast<const AIWorld *>(&world);

        const std::array<WorldPosition, 4> attack_tiles{target.Up(), target.Down(), target.Left(), target.Right()};

    size_t best_distance = std::numeric_limits<size_t>::max();
    std::optional<WorldPosition> best_tile;

    for (const auto& tile: attack_tiles) {
        if (!grid.IsValid(tile))
            continue;
        if (!IsWalkable(grid, tile))
            continue;

            // Can't stand on another occupied agent tile unless it's our own position. (AIWorld only.)
            if (ai_world && ai_world->IsAgentAtPosition(tile, GetID()) &&
                tile != GetLocation().AsWorldPosition()) {
                continue;
            }

        // Use BFS reachability by asking for a move toward that tile.
        const auto move = NextMoveToward(grid, tile);
        if (!move.has_value())
            continue;

        // Full BFS distance
        std::queue<std::pair<WorldPosition, size_t>> frontier;
        std::unordered_set<WorldPosition> visited;
        const WorldPosition start = GetLocation().AsWorldPosition();

        frontier.push({start, 0});
        visited.insert(start);

        bool found = false;
        size_t dist = 0;

        while (!frontier.empty() && !found) {
            auto [pos, d] = frontier.front();
            frontier.pop();

            if (pos == tile) {
                dist = d;
                found = true;
                break;
            }

            const std::array<WorldPosition, 4> neighbors{pos.Up(), pos.Down(), pos.Left(), pos.Right()};

                for (const auto &n: neighbors) {
                    if (visited.contains(n))
                        continue;
                    if (!IsWalkable(grid, n))
                        continue;
                    if (ai_world && ai_world->IsAgentAtPosition(n, GetID()) && !(n == tile))
                        continue;

                visited.insert(n);
                frontier.push({n, d + 1});
            }
        }

        if (found && dist < best_distance) {
            best_distance = dist;
            best_tile = tile;
        }
    }

    if (!best_tile.has_value())
        return std::nullopt;
    return NextMoveToward(grid, *best_tile);
}

/**
 * @brief Selects an exploration move when no attack or pursuit option is available.
 * The exploration strategy prefers neighboring tiles with lower visit counts
 * and penalizes immediately returning to the most recent previous position.
 *
 * @param grid The world grid used to validate candidate moves.
 * @return The action ID for the best exploration move.
 */
std::optional<size_t> SmartEnemyAgent::ExploreMove(const WorldGrid& grid) const {
    const auto* ai_world = dynamic_cast<const AIWorld*>(&world);
    if (!GetLocation().IsPosition())
        return std::nullopt;

    const WorldPosition current = GetLocation().AsWorldPosition();
    const std::array<std::pair<WorldPosition, size_t>, 4> options{{{current.Up(), GetActionID("up")},
                                                                   {current.Down(), GetActionID("down")},
                                                                   {current.Left(), GetActionID("left")},
                                                                   {current.Right(), GetActionID("right")}}};

    size_t best_action = 0;
    size_t best_score = std::numeric_limits<size_t>::max();

    for (const auto& [pos, action]: options) {
        if (!IsWalkable(grid, pos))
            continue;
        if (ai_world && ai_world->IsAgentAtPosition(pos, GetID()))
            continue;

        size_t score = mVisitCounts.contains(pos) ? mVisitCounts.at(pos) : 0;

        if (!mRecentPositions.empty() && pos == mRecentPositions.back()) {
            score += 100;
        }

        if (score < best_score) {
            best_score = score;
            best_action = action;
        }
    }

    if (best_action == 0)
        return std::nullopt;
    return best_action;
}

    /**
     * @brief Selects the smart enemy's next action for the current turn.
     *
     * @details Decision process, in priority order:
     *   1. @b Hunt-radius gate (cross-world): if the world exposes a tracked player
     *      via @ref WorldBase::GetTrackedPlayerPosition() and the player is farther
     *      than Manhattan distance @c 4, skip pursuit and wander via
     *      @ref ExploreMove. This prevents the goblin from telegraphing knowledge
     *      of the player's exact tile from across the dungeon. No-op on worlds
     *      that don't override the hook (the default returns @c std::nullopt).
     *   2. @b Attack if a non-self, targetable agent is adjacent @b and the world
     *      has actually registered an @c attack_* action. The @c attack != 0 guard
     *      matters in worlds like @c DungeonWorld that never register combat
     *      actions: a registered action id of @c 0 means "unknown", and invoking
     *      it would make the agent stall instead of continue moving.
     *   3. @b Pursue the nearest target using BFS line-of-sight or shortest
     *      attack-lane movement.
     *   4. @b Explore the map when no direct pursuit is possible.
     *
     * @param grid The current world grid.
     * @return The selected action ID, or @c 0 if no action is available.
     */
    size_t SmartEnemyAgent::SelectAction(const WorldGrid &grid) {
        if (!GetLocation().IsPosition())
            return 0;

    const WorldPosition current = GetLocation().AsWorldPosition();
    mVisitCounts[current] += 1;
    mRecentPositions.push_back(current);
    if (mRecentPositions.size() > kRecentMemory) {
        mRecentPositions.pop_front();
    }

        // Hunt-radius gate: when a tracked player position is available (DungeonWorld path)
        // and the player is further than Manhattan distance 4, wander instead of chasing.
        // No-op on AIWorld (GetTrackedPlayerPosition returns nullopt there).
        if (const auto tracked = world.GetTrackedPlayerPosition(); tracked.has_value()) {
            const int dist = static_cast<int>(
                    std::abs(static_cast<long>(current.CellX()) - static_cast<long>(tracked->CellX())) +
                    std::abs(static_cast<long>(current.CellY()) - static_cast<long>(tracked->CellY())));
            if (dist > 4) {
                return ExploreMove(grid).value_or(0);
            }
        }

        // 1. Attack immediately if adjacent — but only in worlds that actually register
        //    attack_* actions (e.g. AIWorld). In worlds without attack actions
        //    (e.g. DungeonWorld) GetActionID returns 0, and we skip the attack entirely
        //    so the goblin keeps pursuing/wandering instead of freezing on adjacency.
        if (const auto attack = AttackActionForAdjacentPlayer(); attack.has_value() && *attack != 0) {
            return *attack;
        }

    // 2. Hunt the player.
    if (const auto target = GetTargetPlayerPosition(); target.has_value()) {

        // If visible in same row/col, pursue directly.
        if (const auto los_move = LineOfSightPursuitMove(grid, *target); los_move.has_value()) {
            return *los_move;
        }

        // Otherwise move to the best adjacent attack tile.
        if (const auto lane_move = ShortestAttackLaneMove(grid, *target); lane_move.has_value()) {
            return *lane_move;
        }
    }

    // 3. Fallback exploration.
    const auto fallback = ExploreMove(grid);
    return fallback.value_or(0);
}

} // namespace cse498
