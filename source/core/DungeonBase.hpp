/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>

#include "WorldBase.hpp"
#include "WorldGeneration.hpp"
#include "../tools/WeightedSet.hpp"

namespace cse498 {

  class DungeonBase : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t m_floor_id; ///< Easy access to floor CellType ID.
    size_t m_wall_id;  ///< Easy access to wall CellType ID.
    size_t m_upper_external;
    size_t m_lower_external;
    size_t m_left_external;
    size_t m_right_external;
    size_t m_internal_obstacle;
    size_t m_trap_tile;
    size_t m_loot_tile;
    size_t m_monster_tile;
    size_t m_door_tile;
    size_t m_secret_door;


    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase & agent) override {
      agent.AddAction("up", MOVE_UP);
      agent.AddAction("down", MOVE_DOWN);
      agent.AddAction("left", MOVE_LEFT);
      agent.AddAction("right", MOVE_RIGHT);
    }

  public:
    DungeonBase(const cse498::WeightedSet<std::string>& room_pool) {
      m_floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
      m_wall_id  = main_grid.AddCellType("wall",  "Impenetrable wall.",'#');
      m_upper_external = main_grid.AddCellType("wall",  "upper wall.", '^');
	    m_lower_external = main_grid.AddCellType("wall",  "lower wall.", '&');
      m_left_external = main_grid.AddCellType("wall",  "left external wall.",  '<');
      m_right_external = main_grid.AddCellType("wall",  "right external  wall.", '>');
      m_internal_obstacle = main_grid.AddCellType("wall",  "interal obstacle wall.",   '$');
      m_trap_tile = main_grid.AddCellType("wall",  "trap tile wall.",   't');
      m_loot_tile = main_grid.AddCellType("wall",  "loot tile wall.",    'l');
      m_monster_tile = main_grid.AddCellType("wall",  "monster tile wall.",    'm');
      m_door_tile = main_grid.AddCellType("wall",  "door tile wall.", 'd');
      m_secret_door = main_grid.AddCellType("wall",  "secret tile wall.", 's');

      WorldGeneration generation(room_pool); 
      generation.CreateDungeon(); 
      std::vector<std::string> testing = generation.GetDungeon();

      // Debugging calls in order to see the Grid Parition outline and coordinate/room information
      //generation.GetBSP().TreeParser();
      //generation.GetBSP().GenerateTileMap();

      main_grid.Load(testing);


    }
    ~DungeonBase() = default;

    /// Allow the agents to move around the maze.
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
      if (main_grid[new_position] == m_wall_id
			|| main_grid[new_position] == m_internal_obstacle
			|| main_grid[new_position] == m_upper_external
			|| main_grid[new_position] == m_lower_external
			|| main_grid[new_position] == m_left_external
			|| main_grid[new_position] == m_right_external)
		{ return false; }

      // Set the agent to its new postion.
      agent.SetLocation(new_position);

      return true;
    }

  };

} // End of namespace cse498
