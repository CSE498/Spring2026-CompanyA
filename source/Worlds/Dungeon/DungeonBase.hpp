/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>

#include "../../core/WorldBase.hpp"
#include "WorldGeneration.hpp"
#include "../../tools/WeightedSet.hpp"
#include "../../tools/Random.hpp"

namespace cse498 {

  class DungeonBase : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };
    WorldGeneration m_generation;

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
    size_t m_exit_door;
    size_t m_variant_floor;


    cse498::WeightedSet<std::string> m_item_pool; // A pool of possible items to generate
    


    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase & agent) override {
      agent.AddAction("up", MOVE_UP);
      agent.AddAction("down", MOVE_DOWN);
      agent.AddAction("left", MOVE_LEFT);
      agent.AddAction("right", MOVE_RIGHT);
    }

  public:
    DungeonBase(const cse498::WeightedSet<std::string>& room_pool) : m_generation(room_pool) {
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
      m_exit_door = main_grid.AddCellType("exit",  "secret tile wall.", 'e');
      m_variant_floor = main_grid.AddCellType("wall",  "variant tile wall.", 'v');
      
      auto sword = m_item_pool.Insert("Sword", 1.0);
      auto sword1 = m_item_pool.Insert("Sword +1", 0.2);
      auto sword2 = m_item_pool.Insert("Sword +2", 0.1);
      auto sword3 = m_item_pool.Insert("Sword +3", 0.0);
      auto sword4 = m_item_pool.Insert("Sword +4", 0.0);
      auto sword5 = m_item_pool.Insert("Sword +5", 0.0);
      auto bow = m_item_pool.Insert("Bow", 1.0);
      auto bow1 = m_item_pool.Insert("Bow +1", 0.2);
      auto bow2 = m_item_pool.Insert("Bow +2", 0.1);
      auto bow3 = m_item_pool.Insert("Bow +3", 0.0);
      auto bow4 = m_item_pool.Insert("Bow +4", 0.0);
      auto bow5 = m_item_pool.Insert("Bow +5", 0.0);
      auto healing_potion = m_item_pool.Insert("Healing Potion", 1.0);
      auto defense_potion = m_item_pool.Insert("Defense Potion", 0.5);
      auto speed_potion = m_item_pool.Insert("Speed Potion", 0.5);
      auto axe = m_item_pool.Insert("Axe", 1.0);
      auto pickaxe = m_item_pool.Insert("Pickaxe", 1.0);
      auto shovel = m_item_pool.Insert("Shovel", 1.0);


      m_generation.CreateDungeon(); 
      std::vector<std::string> dungeon = m_generation.GetDungeon();

      // Debugging calls in order to see the Grid Parition outline and coordinate/room information
      //generation.GetBSP().TreeParser();
      m_generation.GetBSP().GenerateTileMap();

      main_grid.Load(dungeon);


    }
    ~DungeonBase() = default;

    /// @brief Grabs user input to determine whether or not to move to the next level. 
    /// Debug tool used for updating level progression + progressing dungeon levels (dungeonone, dungeontwo, dungeonthree)
    void UserInput() { 
      std::cout << "Success! Continue or quit? ('c'/'q')" << std::endl;
      char input;
      bool user_checker = true;
      while(user_checker) {
        std::cin >> input;
        if (std::tolower(input) == 'c') {
          user_checker = false;
        }
        else if (std::tolower(input) == 'q') {
          std::cout << "DungeonGame terminated" << std::endl;
          exit(-1);
        }

        else { 
          std::cout << "Invalid Command!" << std::endl;
          continue;
        }
      }
      return;
    }

    void Update() { 
      m_generation.Update();
      std::vector<std::string> dungeon = m_generation.GetDungeon();

      main_grid.Load(dungeon);
    }

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

        //Here we set up if new_position is walking into an item, they'll pick it up, but not move (LATER)
        //For now we're implementing the 

      else if (main_grid[new_position] == m_exit_door && dynamic_cast<TrashInterface*>(&agent)) { 
        UserInput();
        Update();
        new_position = WorldPosition(1,1); //default player location upon loading into new world

      }
      // Set the agent to its new postion.
      agent.SetLocation(new_position);

      return true;
    }

    /*
    * @brief Random selects an item from a weighted set of possible items
    *
    * @return An randomly selected item
    */
    std::string GetRandomItem(){
      cse498::Random rng;
      double item = rng.GetValue(1.0,m_item_pool.GetTotalWeight()).value();
      return m_item_pool.Sample(item).value();
    }
  };

} // End of namespace cse498
