#pragma once

#include "../core/WorldBase.hpp"
#include "../core/InteractiveWorld/Building.hpp"
#include "../core/InteractiveWorld/InteractiveWorldInventory.hpp"

namespace cse498
{
	class InteractiveWorld : public WorldBase
	{
	protected:

		InteractiveWorldInventory m_inventory;

	    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

	    size_t floor_id; ///< Easy access to floor CellType ID.
	    size_t wall_id;  ///< Easy access to wall CellType ID.

	    /// Provide the agent with movement actions.
	    void ConfigAgent(AgentBase & agent) override {
		    agent.AddAction("up", MOVE_UP);
		    agent.AddAction("down", MOVE_DOWN);
		    agent.AddAction("left", MOVE_LEFT);
		    agent.AddAction("right", MOVE_RIGHT);
	    }

	public:
	    InteractiveWorld()
	    {
		    floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
		    wall_id  = main_grid.AddCellType("wall",  "Impenetrable wall.",             '#');

		    main_grid.Load(std::vector<std::string>{"#######################",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#######################"} );
	    }

	    ~InteractiveWorld() = default;

		InteractiveWorldInventory& GetInventory() {
			return m_inventory;
		}

	    int DoAction(AgentBase & agent, size_t action_id) override {
		    // Determine where the agent is trying to move.
		    WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
		    WorldPosition new_position;
		    switch (action_id) {
			    case REMAIN_STILL: new_position = cur_position; break;
			    case MOVE_UP:      new_position = cur_position.Up(); break;
			    case MOVE_DOWN:    new_position = cur_position.Down(); break;
			    case MOVE_LEFT:    new_position = cur_position.Left(); break;
			    case MOVE_RIGHT:   new_position = cur_position.Right(); break;
		    }

		    // Don't let the agent move off the world or into a wall.
		    if (!main_grid.IsValid(new_position)) { return false; }
		    if (main_grid[new_position] == wall_id) { return false; }

		    // Set the agent to its new position.
		    agent.SetLocation(new_position);

		    return true;
	    }
	};
};
