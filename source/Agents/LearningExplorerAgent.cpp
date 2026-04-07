#include "LearningExplorerAgent.hpp"

#include "../core/WorldBase.hpp"

#include <cmath>
#include <limits>
#include <queue>
#include <unordered_set>

namespace cse498 {
    /**
     * Constructs a LearningExplorerAgent.
     *
     * @param id Unique identifier for the agent.
     * @param name Name of the agent.
     * @param world Reference to the world the agent operates in.
     */
    LearningExplorerAgent::LearningExplorerAgent(size_t id, const std::string &name, const WorldBase &world) :
        AgentBase(id, name, world) {}

    /**
     * Initializes the agent by verifying required movement actions exist.
     *
     * @return True if all required actions ("up", "down", "left", "right") are
     * available.
     */
    bool LearningExplorerAgent::Initialize() {
        return HasAction("up") && HasAction("down") && HasAction("left") && HasAction("right");
    }

    /**
     * Converts a 2D grid position into a 1D cell index.
     *
     * @param grid The world grid.
     * @param pos The position in the grid.
     * @return Linear index corresponding to the position.
     */
    size_t LearningExplorerAgent::CellIndex(const WorldGrid &grid, WorldPosition pos) const {
        return pos.CellY() * grid.GetWidth() + pos.CellX();
    }

    /**
     * Retrieves how many times a cell has been visited.
     *
     * @param grid The world grid.
     * @param pos The position to check.
     * @return Number of visits to the cell (0 if never visited).
     */
    int LearningExplorerAgent::GetVisitCount(const WorldGrid &grid, WorldPosition pos) const {
        const size_t index = CellIndex(grid, pos);
        auto it = m_visit_count_by_cell.find(index);
        return (it == m_visit_count_by_cell.end()) ? 0 : it->second;
    }

    /**
     * Predicts the next position given an action.
     *
     * @param pos Current position.
     * @param action_id Action identifier.
     * @return Resulting position after applying the action.
     */
    WorldPosition LearningExplorerAgent::PredictMove(WorldPosition pos, size_t action_id) const {
        if (action_id == GetActionID("up"))
            return pos.Up();
        if (action_id == GetActionID("down"))
            return pos.Down();
        if (action_id == GetActionID("left"))
            return pos.Left();
        if (action_id == GetActionID("right"))
            return pos.Right();

        return pos;
    }

    /**
     * Updates internal memory of visited cells.
     *
     * Increments visit count for the current position and tracks initial turn
     * state.
     *
     * @param grid The world grid.
     */
    void LearningExplorerAgent::UpdateMemory(const WorldGrid &grid) {
        const WorldPosition current_pos = GetLocation().AsWorldPosition();
        m_visit_count_by_cell[CellIndex(grid, current_pos)]++;

        if (m_first_turn) {
            m_first_turn = false;
        }
    }

    /**
     * Finds the next step toward the nearest unvisited cell using BFS.
     *
     * Avoids walls and cells occupied by other agents.
     *
     * @param grid The world grid.
     * @return The next position to move toward an unvisited cell.
     */
    WorldPosition LearningExplorerAgent::BFSNextStep(const WorldGrid &grid) const {
        const WorldPosition start = GetLocation().AsWorldPosition();
        const size_t start_idx = CellIndex(grid, start);
        const size_t wall_id = grid.GetCellTypeID("wall");

        // Collect cells occupied by other agents so we don't target them.
        std::unordered_set<size_t> agent_cells;
        const auto known = world.GetKnownAgents(*this);
        for (size_t aid: known) {
            if (aid == GetID())
                continue;

            const AgentBase &known_agent = world.GetAgent(aid);

            if (!known_agent.GetLocation().IsPosition())
                continue;

            agent_cells.insert(CellIndex(grid, known_agent.GetLocation().AsWorldPosition()));
        }

        std::queue<WorldPosition> frontier;
        std::unordered_map<size_t, size_t> parent;
        parent[start_idx] = start_idx;
        frontier.push(start);

        while (!frontier.empty()) {
            WorldPosition current = frontier.front();
            frontier.pop();

            size_t cur_idx = CellIndex(grid, current);

            // Target must be unvisited AND not occupied by another agent.
            if (cur_idx != start_idx) {
                auto it = m_visit_count_by_cell.find(cur_idx);

                if (it == m_visit_count_by_cell.end() && agent_cells.find(cur_idx) == agent_cells.end()) {
                    size_t trace = cur_idx;
                    while (parent[trace] != start_idx) {
                        trace = parent[trace];
                    }
                    size_t tx = trace % grid.GetWidth();
                    size_t ty = trace / grid.GetWidth();
                    return WorldPosition{static_cast<double>(tx), static_cast<double>(ty)};
                }
            }

            const WorldPosition neighbors[] = {current.Up(), current.Down(), current.Left(), current.Right()};

            for (const auto &next: neighbors) {
                if (!grid.IsValid(next))
                    continue;

                size_t next_idx = CellIndex(grid, next);

                if (wall_id != 0 && grid[next] == wall_id)
                    continue;
                if (parent.find(next_idx) != parent.end())
                    continue;

                parent[next_idx] = cur_idx;
                frontier.push(next);
            }
        }

        return start;
    }

    /**
     * Rates a potential action based on exploration heuristics.
     *
     * Factors include:
     * - Collision avoidance with walls and agents
     * - Preference for unvisited cells
     * - BFS guidance toward unexplored regions
     * - Anti-oscillation behavior
     *
     * @param grid The world grid.
     * @param action_id The action to evaluate.
     * @param bfs_target Target position from BFS guidance.
     * @return A score representing desirability of the action.
     */
    double LearningExplorerAgent::ScoreAction(const WorldGrid &grid, size_t action_id,
                                              const WorldPosition &bfs_target) const {
        const WorldPosition current_pos = GetLocation().AsWorldPosition();
        const WorldPosition next_pos = PredictMove(current_pos, action_id);

        if (!grid.IsValid(next_pos))
            return BadScore;

        const size_t wall_id = grid.GetCellTypeID("wall");
        if (wall_id != 0 && grid[next_pos] == wall_id)
            return BadScore;

        double score = 0.0;

        // Avoid other agents: heavy penalty for collision, moderate for proximity.
        const auto agent_ids = world.GetKnownAgents(*this);
        for (size_t aid: agent_ids) {
            if (aid == GetID())
                continue;
            const AgentBase &known_agent = world.GetAgent(aid);

            if (!known_agent.GetLocation().IsPosition())
                continue;

            const WorldPosition apos = known_agent.GetLocation().AsWorldPosition();
            const int dx = static_cast<int>(next_pos.CellX()) - static_cast<int>(apos.CellX());
            const int dy = static_cast<int>(next_pos.CellY()) - static_cast<int>(apos.CellY());
            const int dist = std::abs(dx) + std::abs(dy);

            if (dist == 0)
                return BadScore;
            if (dist <= 2)
                score -= AgentProximityPenalty;
        }

        // Strongly prefer unvisited cells; penalise revisits proportionally.
        const int visits = GetVisitCount(grid, next_pos);
        if (visits == 0) {
            score += UnvisitedBonus;
        } else {
            score -= static_cast<double>(visits) * RevisitPenaltyPerVisit;
        }

        // BFS guidance: bonus for the move that leads toward nearest unvisited cell.
        if (!(bfs_target == current_pos) && next_pos == bfs_target) {
            score += BfsGuidanceBonus;
        }

        // Anti-oscillation: penalize going back to the cell we just came from.
        if (m_has_prev_position && next_pos == m_prev_position) {
            score -= OscillationPenalty;
        }

        return score;
    }

    /**
     * Selects the best action based on scoring heuristics.
     *
     * Updates memory, computes BFS target, evaluates all actions,
     * and selects the highest-scoring action.
     *
     * @param grid The world grid.
     * @return The chosen action ID.
     */
    size_t LearningExplorerAgent::SelectAction(const WorldGrid &grid) {
        // Record current cell as visited before choosing next move.
        UpdateMemory(grid);

        // Find the nearest unvisited cell via BFS to guide exploration.
        const WorldPosition bfs_target = BFSNextStep(grid);

        // Gather all candidate movement actions.
        const std::vector<size_t> candidate_actions = {GetActionID("up"), GetActionID("down"), GetActionID("left"),
                                                       GetActionID("right")};

        // Score each candidate and pick the highest-scoring action.
        double best_score = -std::numeric_limits<double>::infinity();
        size_t best_action = 0;

        for (size_t action_id: candidate_actions) {
            if (action_id == 0)
                continue;

            const double score = ScoreAction(grid, action_id, bfs_target);
            if (score > best_score) {
                best_score = score;
                best_action = action_id;
            }
        }

        // Track previous position for anti-oscillation scoring next turn.
        m_prev_position = GetLocation().AsWorldPosition();
        m_has_prev_position = true;

        m_last_action = best_action;
        return best_action;
    }

    /**
     * Returns success count for an action (not implemented).
     */
    int LearningExplorerAgent::GetActionSuccessCount(size_t /* action_id */) const { return 0; }

    /**
     * Returns failure count for an action (not implemented).
     */
    int LearningExplorerAgent::GetActionFailureCount(size_t /* action_id */) const { return 0; }

    /**
     * Returns how many times a cell has been visited.
     *
     * @param grid The world grid.
     * @param pos The position to query.
     * @return Visit count for the cell.
     */
    int LearningExplorerAgent::GetVisitedCellCount(const WorldGrid &grid, WorldPosition pos) const {
        return GetVisitCount(grid, pos);
    }
} // namespace cse498
