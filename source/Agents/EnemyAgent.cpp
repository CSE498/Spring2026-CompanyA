//
// Group1 Enemy agent class
//

#include "EnemyAgent.hpp"

#include "../core/WorldBase.hpp"

#include <limits>

namespace cse498
{
    /**
     * Constructor for the EnemyAgent.
     *
     * Initializes the agent with a unique ID, name, and reference to the world.
     *
     * @param id Unique identifier for the agent.
     * @param name Name of the agent.
     * @param world Reference to the world the agent operates in.
     */
    EnemyAgent::EnemyAgent(size_t id, const std::string &name, const WorldBase &world): AgentBase(id, name, world) {}

    /**
     * Initializes the agent by verifying required movement actions exist.
     *
     * @return True if all required actions are available, false otherwise.
     */
    bool EnemyAgent::Initialize()
    {
        return HasAction("up") && HasAction("down") && HasAction("left") && HasAction("right");
    }

    /**
     * Predicts the resulting position after taking a given action.
     *
     * This function does not modify the agent's state. It simply computes where the agent would end up if the action were taken.
     *
     * @param pos Current position of the agent.
     * @param action_id ID of the action to evaluate.
     * @return The predicted new position after applying the action.
     */
    WorldPosition EnemyAgent::PredictMove(WorldPosition pos, size_t action_id) const
    {
        if (action_id == GetActionID("up")) return pos.Up();
        if (action_id == GetActionID("down")) return pos.Down();
        if (action_id == GetActionID("left")) return pos.Left();
        if (action_id == GetActionID("right")) return pos.Right();
        return pos;
    }

    /**
     * Computes the Manhattan distance between two positions.
     *
     * Manhattan distance is the sum of absolute differences in X and Y, commonly used in grid-based pathfinding.
     *
     * @param a First position.
     * @param b Second position.
     * @return The Manhattan distance between the two positions.
     */
    int EnemyAgent::ManhattanDistance(WorldPosition a, WorldPosition b) const
    {
        const int dx = static_cast<int>(a.CellX()) - static_cast<int>(b.CellX());
        const int dy = static_cast<int>(a.CellY()) - static_cast<int>(b.CellY());
        return std::abs(dx) + std::abs(dy);
    }

    /**
     * Scores a candidate action based on how effectively it moves toward the player.
     *
     * Actions that move closer to the player receive higher scores, while invalid
     * moves (out of bounds or into walls) are heavily penalized.
     *
     * @param grid The current world grid.
     * @param action_id The action being evaluated.
     * @return A score representing the desirability of the action.
     */
    double EnemyAgent::ScoreAction(const WorldGrid &grid, size_t action_id) const
    {
        const WorldPosition current_pos = GetLocation().AsWorldPosition();
        const WorldPosition next_pos = PredictMove(current_pos, action_id);

        // Reject out-of-bounds moves.
        if (!grid.IsValid(next_pos))
        {
            return BadScore;
        }

        // Reject walls.
        const size_t wall_id = grid.GetCellTypeID("wall");
        if (wall_id != 0 && grid[next_pos] == wall_id)
        {
            return BadScore;
        }

		// Get all known agents
		const std::vector<size_t> agent_ids = world.GetKnownAgents(*this);

		 // Default to current position in case no other agent is found.
    	WorldPosition player_pos = current_pos;

    	// Find the first other agent and treat it as the player.
    	for (size_t id : agent_ids)
    	{
        	if (id == GetID())
        	{
            	continue;
        	}

        	const AgentBase &known_agent = world.GetAgent(id);
        	player_pos = known_agent.GetLocation().AsWorldPosition();
        	break;
    	}

        // Smaller distance is better.
        const int dist = ManhattanDistance(next_pos, player_pos);

        // Higher score should mean "better move", so negate distance.
        return -static_cast<double>(dist);
    }

    /**
     * Evaluates all possible movement actions and chooses the one with the highest score based on proximity to the player.
     *
     * @param grid The current world grid.
     * @return The ID of the selected action.
     */
    size_t EnemyAgent::SelectAction(const WorldGrid &grid)
    {
        const std::vector<size_t> candidate_actions =
        {
            GetActionID("up"),
            GetActionID("down"),
            GetActionID("left"),
            GetActionID("right")
        };

        double best_score = -std::numeric_limits<double>::infinity();
        size_t best_action = 0;

        for (size_t action_id : candidate_actions)
        {
            if (action_id == 0)
            {
                continue;
            }

            const double score = ScoreAction(grid, action_id);
            if (score > best_score)
            {
                best_score = score;
                best_action = action_id;
            }
        }

        return best_action;
    }

} // namespace cse498
