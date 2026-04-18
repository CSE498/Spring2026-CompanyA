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

  class DungeonBase : public WorldBase {
    // Constants for item generation
    static constexpr double BASE_RANGE = 1.0;
    static constexpr double BASE_DAMAGE = 1.0;
    static constexpr double BASE_BONUS = 0;
    static constexpr int BASE_GOLD = 3;
    static constexpr size_t MIN_ID = 1;
    static constexpr size_t MAX_ID = 1000000000;

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

    /// Allow the agents to move around the maze and pick up items.
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

      // Generate an item into the player inventory
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
  };

} // End of namespace cse498
