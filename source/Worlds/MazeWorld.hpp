/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>

#include "../core/WorldBase.hpp"
#include "../core/WorldGeneration.hpp"

namespace cse498 {

  class MazeWorld : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id;  ///< Easy access to wall CellType ID.
    size_t upper_external;
    size_t left_external;
    size_t right_external;
    size_t internal_obstacle;
    size_t trap_tile;
    size_t loot_tile;
    size_t monster_tile;
    size_t door_tile;
    size_t secret_door;


    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase & agent) override {
      agent.AddAction("up", MOVE_UP);
      agent.AddAction("down", MOVE_DOWN);
      agent.AddAction("left", MOVE_LEFT);
      agent.AddAction("right", MOVE_RIGHT);
    }

  public:
    MazeWorld() {
      floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
      wall_id  = main_grid.AddCellType("wall",  "Impenetrable wall.",'#');
      upper_external = main_grid.AddCellType("wall",  "upper wall.", '^');
      left_external = main_grid.AddCellType("wall",  "left external wall.",  '<');
      right_external = main_grid.AddCellType("wall",  "right external  wall.", '>');
      internal_obstacle = main_grid.AddCellType("wall",  "interal obstacle wall.",   '$');
      trap_tile = main_grid.AddCellType("wall",  "trap tile wall.",   't');
      loot_tile = main_grid.AddCellType("wall",  "loot tile wall.",    'l');
      monster_tile = main_grid.AddCellType("wall",  "monster tile wall.",    'm');
      door_tile = main_grid.AddCellType("wall",  "door tile wall.", 'd');
      secret_door = main_grid.AddCellType("wall",  "secret tile wall.", 's');



      //Original level commented out
      // main_grid.Load(std::vector<std::string>{"#######################",
      //                                         "# #            ##     #",
      //                                         "# #  #  ######    ### #",
      //                                         "# #  #  #     #  #  # #",
      //                                         "# #  #  #  #  #  #  # #",
      //                                         "#    #     #     #    #",
      //                                         "##################  # #",
      //                                         "#                    ##",
      //                                         "#                    ##",
      //                                         "#  ####################",
      //                                         "#######################"} );


      ////////////////////////////////////
      //    BSP-Dungeon Implementation
      ///////////////////////////////////

      WorldGen generation; 
      generation.CreateDungeon(); 
      std::vector<std::string> testing = generation.GetDungeon();

      // Debugging calls in order to see the Grid Parition outline and coordinate/room information
      // generation.GetBSP().TreeParser();
      // generation.GetBSP().GenerateTileMap();

      main_grid.Load(testing);


    }
    ~MazeWorld() = default;

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
      if (main_grid[new_position] == wall_id) { return false; }

      // Set the agent to its new postion.
      agent.SetLocation(new_position);

      return true;
    }

  };

} // End of namespace cse498
