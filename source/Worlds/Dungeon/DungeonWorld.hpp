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
#include "ForestLevel.hpp"
#include "CaveLevel.hpp"
#include "CastleLevel.hpp"
#include "LevelBase.hpp"
#include "../../Interfaces/TrashInterface.hpp"

#include "../../core/item/ItemWeaponSword.hpp"
#include "../../core/item/ItemWeaponBow.hpp"
#include "../../core/item/ItemConsumableDefense.hpp"
#include "../../core/item/ItemConsumableHealing.hpp"
#include "../../core/item/ItemConsumableSpeed.hpp"
#include "../../core/item/ItemWeaponToolAxe.hpp"
#include "../../core/item/ItemWeaponToolPickaxe.hpp"
#include "../../core/item/ItemWeaponToolShovel.hpp"
#include "../../Agents/Classic/PlayerFeatures/Inventory.hpp"

namespace cse498 {

  class DungeonWorld : public WorldBase {
	static constexpr double BASE_RANGE = 1.0;
	static constexpr double BASE_DAMAGE = 1.0;
	static constexpr double BASE_BONUS = 0;
	static constexpr int BASE_GOLD = 3;
	static constexpr size_t MIN_ID = 1;
	static constexpr size_t MAX_ID = 1000000000;

	private:
		int m_level_num = 1; //Current int value level that the game is on
		std::unique_ptr<LevelBase> m_level = std::make_unique<ForestLevel>(); //The currently pointed to level that the player agent is on

		/**
		 * @brief Builds the room pool for the current level.
		 * @return WeightedSet of room file paths and weights.
		 */
		void LoadLevelData() {
			std::cout << "Currently on level: " << m_level_num << std::endl;
			switch (m_level_num) {
				
				case 1:
					m_level = std::make_unique<ForestLevel>();
					break;

				case 2:
					m_level = std::make_unique<CaveLevel>();
					break;

				case 3: 
					m_level = std::make_unique<CastleLevel>();
					break;
					
				default:
					m_level = std::make_unique<ForestLevel>();
					break;
			}
		}

  	protected:
		enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };
		WorldGeneration m_generation;

		size_t m_floor_id_l1v1; ///< Level 1 floor CellType IDs
		size_t m_floor_id_l1v2;
		size_t m_floor_id_l1v3;
		size_t m_floor_id_l1v4;
		size_t m_floor_id_l1v5;
		size_t m_floor_id_l2v1; ///< Level 2 floor CellType IDs
		size_t m_floor_id_l2v2;
		size_t m_floor_id_l2v3;
		size_t m_floor_id_l2v4;
		size_t m_floor_id_l2v5;
		size_t m_floor_id_l3v1; ///< Level 3 floor CellType IDs
		size_t m_floor_id_l3v2;
		size_t m_floor_id_l3v3;
		size_t m_floor_id_l3v4;
		size_t m_floor_id_l3v5;
		size_t m_floor_id_l4v1; ///< Level 4 floor CellType IDs
		size_t m_floor_id_l4v2;
		size_t m_floor_id_l4v3;
		size_t m_floor_id_l4v4;
		size_t m_floor_id_l4v5;
		
		size_t m_wall_id;  ///< Easy access to wall CellType ID.
		
		size_t m_upper_external_l1; ///< Level 1 wall CellType IDs
		size_t m_lower_external_l1;
		size_t m_left_external_l1;
		size_t m_right_external_l1;
		size_t m_internal_obstacle_l1v1;
		size_t m_internal_obstacle_l1v2;
		size_t m_door_tile_left_l1;
		size_t m_door_tile_right_l1;
		size_t m_upper_external_l2; ///< Level 2 wall CellType IDs
		size_t m_lower_external_l2;
		size_t m_left_external_l2;
		size_t m_right_external_l2;
		size_t m_internal_obstacle_l2v1;
		size_t m_internal_obstacle_l2v2;
		size_t m_door_tile_left_l2;
		size_t m_door_tile_right_l2;
		size_t m_upper_external_l3; ///< Level 3 wall CellType IDs
		size_t m_lower_external_l3;
		size_t m_left_external_l3;
		size_t m_right_external_l3;
		size_t m_internal_obstacle_l3v1;
		size_t m_internal_obstacle_l3v2;
		size_t m_door_tile_left_l3;
		size_t m_door_tile_right_l3;
		size_t m_upper_external_l4; ///< Level 4 wall CellType IDs
		size_t m_lower_external_l4;
		size_t m_left_external_l4;
		size_t m_right_external_l4;
		size_t m_internal_obstacle_l4v1;
		size_t m_internal_obstacle_l4v2;
		size_t m_door_tile_left_l4;
		size_t m_door_tile_right_l4;
		
		size_t m_trap_tile;
		size_t m_loot_tile;
		size_t m_monster_tile_skeleton;
		size_t m_monster_tile_goblin;
		size_t m_secret_door;
		size_t m_exit_door;
		//size_t m_variant_floor;

		cse498::WeightedSet<std::string> m_item_pool; // A pool of possible items to generate

		/// Provide the agent with movement actions.
		void ConfigAgent(AgentBase & agent) override {
			agent.AddAction("up", MOVE_UP);
			agent.AddAction("down", MOVE_DOWN);
			agent.AddAction("left", MOVE_LEFT);
			agent.AddAction("right", MOVE_RIGHT);
		}

  	public:
		DungeonWorld() : m_generation(*m_level) {
			// Level 1 floors
			m_floor_id_l1v1 = main_grid.AddCellType("dun_floor_l1v1", "Floor that agents can walk on. l1 v1", 'a');
			m_floor_id_l1v2 = main_grid.AddCellType("dun_floor_l1v2", "Floor that agents can walk on. l1 v2", 'b');
			m_floor_id_l1v3 = main_grid.AddCellType("dun_floor_l1v3", "Floor that agents can walk on. l1 v3", 'c');
			m_floor_id_l1v4 = main_grid.AddCellType("dun_floor_l1v4", "Floor that agents can walk on. l1 v4", 'd');
			m_floor_id_l1v5 = main_grid.AddCellType("dun_floor_l1v5", "Floor that agents can walk on. l1 v5", '<');
			// Level 2 floors
			m_floor_id_l2v1 = main_grid.AddCellType("dun_floor_l2v1", "Floor that agents can walk on. l2 v1", 'A');
			m_floor_id_l2v2 = main_grid.AddCellType("dun_floor_l2v2", "Floor that agents can walk on. l2 v2", 'B');
			m_floor_id_l2v3 = main_grid.AddCellType("dun_floor_l2v3", "Floor that agents can walk on. l2 v3", 'C');
			m_floor_id_l2v4 = main_grid.AddCellType("dun_floor_l2v4", "Floor that agents can walk on. l2 v4", 'D');
			m_floor_id_l2v5 = main_grid.AddCellType("dun_floor_l2v5", "Floor that agents can walk on. l2 v5", 'E');
			// Level 3 floors
			m_floor_id_l3v1 = main_grid.AddCellType("dun_floor_l3v1", "Floor that agents can walk on. l3 v1", 'm');
			m_floor_id_l3v2 = main_grid.AddCellType("dun_floor_l3v2", "Floor that agents can walk on. l3 v2", 'n');
			m_floor_id_l3v3 = main_grid.AddCellType("dun_floor_l3v3", "Floor that agents can walk on. l3 v3", 'o');
			m_floor_id_l3v4 = main_grid.AddCellType("dun_floor_l3v4", "Floor that agents can walk on. l3 v4", 'p');
			m_floor_id_l3v5 = main_grid.AddCellType("dun_floor_l3v5", "Floor that agents can walk on. l3 v5", 'q');
			// Level 4 floors
			m_floor_id_l4v1 = main_grid.AddCellType("dun_floor_l4v1", "Floor that agents can walk on. l4 v1", 'M');
			m_floor_id_l4v2 = main_grid.AddCellType("dun_floor_l4v2", "Floor that agents can walk on. l4 v2", 'N');
			m_floor_id_l4v3 = main_grid.AddCellType("dun_floor_l4v3", "Floor that agents can walk on. l4 v3", 'O');
			m_floor_id_l4v4 = main_grid.AddCellType("dun_floor_l4v4", "Floor that agents can walk on. l4 v4", 'P');
			m_floor_id_l4v5 = main_grid.AddCellType("dun_floor_l4v5", "Floor that agents can walk on. l4 v5", 'Q');

			m_wall_id = main_grid.AddCellType("dun_wall", "Impenetrable wall.", '#');

			// Level 1 walls
			m_upper_external_l1 = main_grid.AddCellType("dun_wall_top_l1", "upper wall. l1", '1');
			m_lower_external_l1 = main_grid.AddCellType("dun_wall_bottom_l1", "lower wall. l1", '2');
			m_left_external_l1 = main_grid.AddCellType("dun_wall_left_l1", "left external wall. l1", '3');
			m_right_external_l1 = main_grid.AddCellType("dun_wall_right_l1", "right external wall. l1", '4');
			m_internal_obstacle_l1v1 = main_grid.AddCellType("dun_wall_internal_l1v1", "internal obstacle wall. l1 v1", '5');
			m_internal_obstacle_l1v2 = main_grid.AddCellType("dun_wall_internal_l1v2", "internal obstacle wall. l1 v2", '6');
			m_door_tile_left_l1 = main_grid.AddCellType("dun_door_left_l1", "left door tile. l1", '7');
			m_door_tile_right_l1 = main_grid.AddCellType("dun_door_right_l1", "right door tile. l1", '8');
			// Level 2 walls
			m_upper_external_l2 = main_grid.AddCellType("dun_wall_top_l2", "upper wall. l2", '!');
			m_lower_external_l2 = main_grid.AddCellType("dun_wall_bottom_l2", "lower wall. l2", '@');
			m_left_external_l2 = main_grid.AddCellType("dun_wall_left_l2", "left external wall. l2", '?');
			m_right_external_l2 = main_grid.AddCellType("dun_wall_right_l2", "right external wall. l2", '$');
			m_internal_obstacle_l2v1 = main_grid.AddCellType("dun_wall_internal_l2v1", "internal obstacle wall. l2 v1", '%');
			m_internal_obstacle_l2v2 = main_grid.AddCellType("dun_wall_internal_l2v2", "internal obstacle wall. l2 v2", '^');
			m_door_tile_left_l2 = main_grid.AddCellType("dun_door_left_l2", "left door tile. l2", '&');
			m_door_tile_right_l2 = main_grid.AddCellType("dun_door_right_l2", "right door tile. l2", '*');
			// Level 3 walls
			m_upper_external_l3 = main_grid.AddCellType("dun_wall_top_l3", "upper wall. l3", '9');
			m_lower_external_l3 = main_grid.AddCellType("dun_wall_bottom_l3", "lower wall. l3", '0');
			m_left_external_l3 = main_grid.AddCellType("dun_wall_left_l3", "left external wall. l3", '-');
			m_right_external_l3 = main_grid.AddCellType("dun_wall_right_l3", "right external wall. l3", '=');
			m_internal_obstacle_l3v1 = main_grid.AddCellType("dun_wall_internal_l3v1", "internal obstacle wall. l3 v1", '[');
			m_internal_obstacle_l3v2 = main_grid.AddCellType("dun_wall_internal_l3v2", "internal obstacle wall. l3 v2", ']');
			m_door_tile_left_l3 = main_grid.AddCellType("dun_door_left_l3", "left door tile. l3", '.');
			m_door_tile_right_l3 = main_grid.AddCellType("dun_door_right_l3", "right door tile. l3", ';');
			// Level 4 walls
			m_upper_external_l4 = main_grid.AddCellType("dun_wall_top_l4", "upper wall. l4", '(');
			m_lower_external_l4 = main_grid.AddCellType("dun_wall_bottom_l4", "lower wall. l4", ')');
			m_left_external_l4 = main_grid.AddCellType("dun_wall_left_l4", "left external wall. l4", '_');
			m_right_external_l4 = main_grid.AddCellType("dun_wall_right_l4", "right external wall. l4", '+');
			m_internal_obstacle_l4v1 = main_grid.AddCellType("dun_wall_internal_l4v1", "internal obstacle wall. l4 v1", '{');
			m_internal_obstacle_l4v2 = main_grid.AddCellType("dun_wall_internal_l4v2", "internal obstacle wall. l4 v2", '}');
			m_door_tile_left_l4 = main_grid.AddCellType("dun_door_left_l4", "left door tile. l4", '~');
			m_door_tile_right_l4 = main_grid.AddCellType("dun_door_right_l4", "right door tile. l4", ':');

			// Shared special tiles
			m_trap_tile = main_grid.AddCellType("dun_trap", "trap tile.", 't');
			m_loot_tile = main_grid.AddCellType("dun_loot", "loot tile.", 'l');
			m_monster_tile_skeleton = main_grid.AddCellType("dun_monster_skeleton", "monster tile. skeleton", 's');
			m_monster_tile_goblin = main_grid.AddCellType("dun_monster_goblin", "monster tile. goblin", 'g');
			m_secret_door = main_grid.AddCellType("dun_secret_door", "secret door tile.", 'f');
			m_exit_door = main_grid.AddCellType("dun_exit_door", "exit door tile.", 'e');

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

			//   WorldGeneration generation(room_pool); 
			//   generation.CreateDungeon(); 
			//   std::vector<std::string> testing = generation.GetDungeon();

			// Debugging calls in order to see the Grid Parition outline and coordinate/room information
			//generation.GetBSP().TreeParser();
			//generation.GetBSP().GenerateTileMap();

			//   main_grid.Load(testing);

			GenerateLevel();
		}

		~DungeonWorld() = default;

		/// @brief Generates the DungeonWorld based on the currently selected level from m_level
		void GenerateLevel() {
			LoadLevelData();
			m_generation.CreateDungeon(m_level_num);
			main_grid.Load(m_generation.GetDungeon());
		}

    	void AdvanceLevel();

		// void UpdateWorld() override {
		// 	if ((m_level_num == 5)) {
        // 		mRunOver = true;
		// 		return
    	// 	}
			
		// 	if (player_is_on_exit) {
		// 		AdvanceLevel();
		// 	}
		// }


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
			++m_level_num;
			m_generation.Update(m_level_num);
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
			|| main_grid[new_position] == m_lower_external_l1
			|| main_grid[new_position] == m_left_external_l1
			|| main_grid[new_position] == m_right_external_l1
			|| main_grid[new_position] == m_internal_obstacle_l1v1
			|| main_grid[new_position] == m_internal_obstacle_l1v2

			|| main_grid[new_position] == m_lower_external_l2
			|| main_grid[new_position] == m_left_external_l2
			|| main_grid[new_position] == m_right_external_l2
			|| main_grid[new_position] == m_internal_obstacle_l2v1
			|| main_grid[new_position] == m_internal_obstacle_l2v2
      
			|| main_grid[new_position] == m_lower_external_l3
			|| main_grid[new_position] == m_left_external_l3
			|| main_grid[new_position] == m_right_external_l3
			|| main_grid[new_position] == m_internal_obstacle_l3v1
			|| main_grid[new_position] == m_internal_obstacle_l3v2
      
			|| main_grid[new_position] == m_lower_external_l4
			|| main_grid[new_position] == m_left_external_l4
			|| main_grid[new_position] == m_right_external_l4
			|| main_grid[new_position] == m_internal_obstacle_l4v1
			|| main_grid[new_position] == m_internal_obstacle_l4v2)
			{ return false; }

		if (main_grid[new_position] == m_loot_tile && agent.GetID() == 0) {
        
        if (auto* player = dynamic_cast<PlayerAgent*>(&agent))
        {
          Inventory & inventory = player->GetInventory();
          std::string item_str = GetRandomItem();
          Random rng;
          size_t item_id = rng.GetValue(MIN_ID,MAX_ID).value();
          
          if (item_str == "Sword") {
            std::unique_ptr<ItemWeaponSword> sword = std::make_unique<ItemWeaponSword>(
              item_id,
              item_str,
              "/assets/item/item_sword_1.png",
              BASE_GOLD + 1,
              *this
            );
            sword->SetRange(BASE_RANGE);
            sword->SetDamage(BASE_DAMAGE + 0.5);
            sword->SetHitBonus(BASE_BONUS);

            inventory.AddItem(std::move(sword));
			}
			else if (item_str == "Sword +1") {
				std::unique_ptr<ItemWeaponSword> sword1 = std::make_unique<ItemWeaponSword>(
				item_id,
				item_str,
				"/assets/item/item_sword_1.png",
				BASE_GOLD + 2,
				*this
				);
				sword1->SetRange(BASE_RANGE);
				sword1->SetDamage(BASE_DAMAGE + 1.0);
				sword1->SetHitBonus(BASE_BONUS + 1);

				inventory.AddItem(std::move(sword1));
			}
			else if (item_str == "Sword +2") {
				std::unique_ptr<ItemWeaponSword> sword2 = std::make_unique<ItemWeaponSword>(
				item_id,
				item_str,
				"/assets/item/item_sword_1.png",
				BASE_GOLD + 3,
				*this
				);
				sword2->SetRange(BASE_RANGE);
				sword2->SetDamage(BASE_DAMAGE + 1.5);
				sword2->SetHitBonus(BASE_BONUS + 2);

				inventory.AddItem(std::move(sword2));
			}
			else if (item_str == "Sword +3") {      
				std::unique_ptr<ItemWeaponSword> sword3 = std::make_unique<ItemWeaponSword>(
				item_id,
				item_str,
				"/assets/item/item_sword_1.png",
				BASE_GOLD + 4,
				*this
				);
				sword3->SetRange(BASE_RANGE);
				sword3->SetDamage(BASE_DAMAGE + 2.0);
				sword3->SetHitBonus(BASE_BONUS + 3);

				inventory.AddItem(std::move(sword3));
			}
			else if (item_str == "Sword +4") {
				std::unique_ptr<ItemWeaponSword> sword4 = std::make_unique<ItemWeaponSword>(
				item_id,
				item_str,
				"/assets/item/item_sword_1.png",
				BASE_GOLD + 5,
				*this
				);
				sword4->SetRange(BASE_RANGE);
				sword4->SetDamage(BASE_DAMAGE + 2.5);
				sword4->SetHitBonus(BASE_BONUS + 4);

				inventory.AddItem(std::move(sword4));
			}
			else if (item_str == "Sword +5") {
				std::unique_ptr<ItemWeaponSword> sword5 = std::make_unique<ItemWeaponSword>(
				item_id,
				item_str,
				"/assets/item/item_sword_1.png",
				BASE_GOLD + 6,
				*this
				);
				sword5->SetRange(BASE_RANGE);
				sword5->SetDamage(BASE_DAMAGE + 3.0);
				sword5->SetHitBonus(BASE_BONUS + 5);

				inventory.AddItem(std::move(sword5));
			}
			
			if (item_str == "Bow") {
				std::unique_ptr<ItemWeaponBow> bow = std::make_unique<ItemWeaponBow>(
				item_id,
				item_str,
				"/assets/item/item_bow_1.png",
				BASE_GOLD + 1,
				*this
				);
				bow->SetRange(BASE_RANGE+3);
				bow->SetDamage(BASE_DAMAGE + 0.5);
				bow->SetHitBonus(BASE_BONUS);

				inventory.AddItem(std::move(bow));
			}
			else if (item_str == "Bow +1") {
				std::unique_ptr<ItemWeaponBow> bow1 = std::make_unique<ItemWeaponBow>(
				item_id,
				item_str,
				"/assets/item/item_bow_1.png",
				BASE_GOLD + 2,
				*this
				);
				bow1->SetRange(BASE_RANGE+3);
				bow1->SetDamage(BASE_DAMAGE + 1.0);
				bow1->SetHitBonus(BASE_BONUS + 1);

				inventory.AddItem(std::move(bow1));
			}
			else if (item_str == "Bow +2") {
				std::unique_ptr<ItemWeaponBow> bow2 = std::make_unique<ItemWeaponBow>(
				item_id,
				item_str,
				"/assets/item/item_bow_1.png",
				BASE_GOLD + 3,
				*this
				);
				bow2->SetRange(BASE_RANGE+3);
				bow2->SetDamage(BASE_DAMAGE + 1.5);
				bow2->SetHitBonus(BASE_BONUS + 2);

				inventory.AddItem(std::move(bow2));
			}
			else if (item_str == "Bow +3") {
				std::unique_ptr<ItemWeaponBow> bow3 = std::make_unique<ItemWeaponBow>(
				item_id,
				item_str,
				"/assets/item/item_bow_1.png",
				BASE_GOLD + 4,
				*this
				);
				bow3->SetRange(BASE_RANGE+3);
				bow3->SetDamage(BASE_DAMAGE + 2.0);
				bow3->SetHitBonus(BASE_BONUS + 3);

				inventory.AddItem(std::move(bow3));
			}
			else if (item_str == "Bow +4") {
				std::unique_ptr<ItemWeaponBow> bow4 = std::make_unique<ItemWeaponBow>(
				item_id,
				item_str,
				"/assets/item/item_bow_1.png",
				BASE_GOLD + 5,
				*this
				);
				bow4->SetRange(BASE_RANGE+3);
				bow4->SetDamage(BASE_DAMAGE + 2.5);
				bow4->SetHitBonus(BASE_BONUS + 4);

				inventory.AddItem(std::move(bow4));
			}
			else if (item_str == "Bow +5") {
				std::unique_ptr<ItemWeaponBow> bow5 = std::make_unique<ItemWeaponBow>(
				item_id,
				item_str,
				"/assets/item/item_bow_1.png",
				BASE_GOLD + 6,
				*this
				);
				bow5->SetRange(BASE_RANGE+4);
				bow5->SetDamage(BASE_DAMAGE + 3.0);
				bow5->SetHitBonus(BASE_BONUS + 5);

				inventory.AddItem(std::move(bow5));
			}
			
			else if (item_str == "Healing Potion") {
				std::unique_ptr<ItemConsumableHealing> healing = std::make_unique<ItemConsumableHealing>(
				item_id,
				item_str,
				"/assets/item/potion_healing.png",
				BASE_GOLD,
				*this
				);
				healing->SetCharges(1);
				healing->SetDuration(0);
				
				inventory.AddItem(std::move(healing));
			}
			else if (item_str == "Defense Potion") {
				std::unique_ptr<ItemConsumableDefense> defense = std::make_unique<ItemConsumableDefense>(
				item_id,
				item_str,
				"/assets/item/potion_defense.png",
				BASE_GOLD,
				*this
				);
				defense->SetCharges(1);
				defense->SetDuration(3);
				
				inventory.AddItem(std::move(defense));
			}
			else if (item_str == "Speed Potion") {
				std::unique_ptr<ItemConsumableSpeed> speed = std::make_unique<ItemConsumableSpeed>(
				item_id,
				item_str,
				"/assets/item/potion_speed.png",
				BASE_GOLD,
				*this
				);
				speed->SetCharges(1);
				speed->SetDuration(3);
				
				inventory.AddItem(std::move(speed));
			}
			
			else if (item_str == "Axe") {
				std::unique_ptr<ItemWeaponToolAxe> axe = std::make_unique<ItemWeaponToolAxe>(
				item_id,
				item_str,
				"/assets/item/item_axe.png",
				BASE_GOLD,
				*this
				);
				axe->SetRange(BASE_RANGE);
				axe->SetDamage(BASE_DAMAGE);
				axe->SetHitBonus(BASE_BONUS);
				axe->SetDropRate(1);
				axe->SetHarvestSpeed(1);

				inventory.AddItem(std::move(axe));
			}
			else if (item_str == "Pickaxe") {
				std::unique_ptr<ItemWeaponToolPickaxe> pickaxe = std::make_unique<ItemWeaponToolPickaxe>(
				item_id,
				item_str,
				"/assets/item/item_pickaxe.png",
				BASE_GOLD,
				*this
				);
				pickaxe->SetRange(BASE_RANGE);
				pickaxe->SetDamage(BASE_DAMAGE);
				pickaxe->SetHitBonus(BASE_BONUS);
				pickaxe->SetDropRate(1);
				pickaxe->SetHarvestSpeed(1);

				inventory.AddItem(std::move(pickaxe));
			}
			else if (item_str == "Shovel") {
				std::unique_ptr<ItemWeaponToolPickaxe> shovel = std::make_unique<ItemWeaponToolPickaxe>(
				item_id,
				item_str,
				"/assets/item/item_shovel.png",
				BASE_GOLD,
				*this
				);
				shovel->SetRange(BASE_RANGE);
				shovel->SetDamage(BASE_DAMAGE);
				shovel->SetHitBonus(BASE_BONUS);
				shovel->SetDropRate(1);
				shovel->SetHarvestSpeed(1);

				inventory.AddItem(std::move(shovel));
			}
			}
		}

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
