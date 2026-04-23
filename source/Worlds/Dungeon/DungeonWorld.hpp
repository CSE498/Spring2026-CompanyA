/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A World that consists only of walls and open cells.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <array>
#include <memory>
#include <vector>

#include "../../core/WorldBase.hpp"
#include "WorldGeneration.hpp"
#include "../../tools/WeightedSet.hpp"
#include "ForestLevel.hpp"
#include "CaveLevel.hpp"
#include "CastleLevel.hpp"
#include "LevelBase.hpp"
#include "../../Interfaces/TrashInterface.hpp"
#include "../../Agents/PacingAgent.hpp"
#include "../../Agents/Classic/Enemy.hpp"

#include "../../core/item/ItemWeaponSword.hpp"
#include "../../core/item/ItemWeaponBow.hpp"
#include "../../core/item/ItemConsumableDefense.hpp"
#include "../../core/item/ItemConsumableHealing.hpp"
#include "../../core/item/ItemConsumableSpeed.hpp"
#include "../../core/item/ItemWeaponToolAxe.hpp"
#include "../../Worlds/DemoG2/WorldActions.hpp"
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
        std::unique_ptr<LevelBase> m_level = std::make_unique<ForestLevel>();

        //The currently pointed to level that the player agent is on
		std::vector<size_t> m_spawned_enemy_ids;

        size_t m_player_id;
        size_t m_enemy_id;

		//Track level change to prevent an extra enemy turn
		bool mLevelJustAdvanced = false;
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
        enum ActionType { REMAIN_STILL = 0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

        std::unique_ptr<WorldGeneration> m_generation;

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
		/// Level 4 floor CellType IDs-> not used currently
		// size_t m_floor_id_l4v1; 
        // size_t m_floor_id_l4v2;
        // size_t m_floor_id_l4v3;
        // size_t m_floor_id_l4v4;
        // size_t m_floor_id_l4v5;

        size_t m_wall_id; ///< Easy access to wall CellType ID.

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
		/// Level 4 wall CellType IDs -> not used currently
        //size_t m_upper_external_l4;
        // size_t m_lower_external_l4;
        // size_t m_left_external_l4;
        // size_t m_right_external_l4;
        // size_t m_internal_obstacle_l4v1;
        // size_t m_internal_obstacle_l4v2;
        // size_t m_door_tile_left_l4;
        // size_t m_door_tile_right_l4;

        size_t m_trap_tile;
        size_t m_loot_tile;
        size_t m_monster_tile_skeleton;
        size_t m_monster_tile_goblin;
        size_t m_secret_door_top;
        size_t m_secret_door_bottom;
        size_t m_secret_door_left;
        size_t m_secret_door_right;
        size_t m_exit_door_l1;
        size_t m_exit_door_l2;
        size_t m_exit_door_l3;
        size_t m_exit_door_l4;
        //size_t m_variant_floor;

        cse498::WeightedSet<std::string> m_item_pool; // A pool of possible items to generate

        /// Provide the agent with movement actions.
        void ConfigAgent(AgentBase &agent) override {
            agent.AddAction("up", MOVE_UP);
            agent.AddAction("down", MOVE_DOWN);
            agent.AddAction("left", MOVE_LEFT);
            agent.AddAction("right", MOVE_RIGHT);
            agent.AddAction(WorldActions::INTERACT_STRING, WorldActions::INTERACT);
            agent.AddAction(WorldActions::REMAIN_STILL_STRING, WorldActions::REMAIN_STILL);
        }

    public:
        DungeonWorld() {
            m_floor_id_l1v1 = main_grid.AddCellType("floor_l1v1", "Floor that agents can walk on. l1 v1", 'a');
            m_floor_id_l1v2 = main_grid.AddCellType("floor_l1v2", "Floor that agents can walk on. l1 v2", 'b');
            m_floor_id_l1v3 = main_grid.AddCellType("floor_l1v3", "Floor that agents can walk on. l1 v3", 'c');
            m_floor_id_l1v4 = main_grid.AddCellType("floor_l1v4", "Floor that agents can walk on. l1 v4", 'd');
            m_floor_id_l1v5 = main_grid.AddCellType("floor_l1v5", "Floor that agents can walk on. l1 v5", '<');
            m_floor_id_l2v1 = main_grid.AddCellType("floor_l2v1", "Floor that agents can walk on. l2 v1", 'A');
            m_floor_id_l2v2 = main_grid.AddCellType("floor_l2v2", "Floor that agents can walk on. l2 v2", 'B');
            m_floor_id_l2v3 = main_grid.AddCellType("floor_l2v3", "Floor that agents can walk on. l2 v3", 'C');
            m_floor_id_l2v4 = main_grid.AddCellType("floor_l2v4", "Floor that agents can walk on. l2 v4", 'D');
            m_floor_id_l2v5 = main_grid.AddCellType("floor_l2v5", "Floor that agents can walk on. l2 v5", 'E');
            m_floor_id_l3v1 = main_grid.AddCellType("floor_l3v1", "Floor that agents can walk on. l3 v1", 'm');
            m_floor_id_l3v2 = main_grid.AddCellType("floor_l3v2", "Floor that agents can walk on. l3 v2", 'n');
            m_floor_id_l3v3 = main_grid.AddCellType("floor_l3v3", "Floor that agents can walk on. l3 v3", 'o');
            m_floor_id_l3v4 = main_grid.AddCellType("floor_l3v4", "Floor that agents can walk on. l3 v4", 'p');
            m_floor_id_l3v5 = main_grid.AddCellType("floor_l3v5", "Floor that agents can walk on. l3 v5", 'q');
            // m_floor_id_l4v1 = main_grid.AddCellType("floor_l4v1", "Floor that agents can walk on. l4 v1", 'M');
            // m_floor_id_l4v2 = main_grid.AddCellType("floor_l4v2", "Floor that agents can walk on. l4 v2", 'N');
            // m_floor_id_l4v3 = main_grid.AddCellType("floor_l4v3", "Floor that agents can walk on. l4 v3", 'O');
            // m_floor_id_l4v4 = main_grid.AddCellType("floor_l4v4", "Floor that agents can walk on. l4 v4", 'P');
            // m_floor_id_l4v5 = main_grid.AddCellType("floor_l4v5", "Floor that agents can walk on. l4 v5", 'Q');

            m_wall_id = main_grid.AddCellType("wall", "Impenetrable wall.", '#');

            m_upper_external_l1 = main_grid.AddCellType("wall_l1v1", "upper wall. l1", '1');
            m_lower_external_l1 = main_grid.AddCellType("wall_l1v2", "lower wall. l1", '2');
            m_left_external_l1 = main_grid.AddCellType("wall_l1v13", "left external wall. l1", '3');
            m_right_external_l1 = main_grid.AddCellType("wall_l1v4", "right external  wall. l1", '4');
            m_internal_obstacle_l1v1 = main_grid.AddCellType("wall_l1v5", "interal obstacle wall. l1 v1", '5');
            m_internal_obstacle_l1v2 = main_grid.AddCellType("wall_l1v6", "interal obstacle wall. l1 v2", '6');
            m_door_tile_left_l1 = main_grid.AddCellType("wall_l1v7", "left door tile wall. l1", '7');
            m_door_tile_right_l1 = main_grid.AddCellType("wall_l1v8", "right door tile wall. l1", '8');
            m_upper_external_l2 = main_grid.AddCellType("wall_l2v1", "upper wall. l2", '!');
            m_lower_external_l2 = main_grid.AddCellType("wall_l2v2", "lower wall. l2", '@');
            m_left_external_l2 = main_grid.AddCellType("wall_l2v3", "left external wall. l2", '?');
            m_right_external_l2 = main_grid.AddCellType("wall_l2v4", "right external  wall. l2", '$');
            m_internal_obstacle_l2v1 = main_grid.AddCellType("wall_l2v5", "interal obstacle wall. l2 v1", '%');
            m_internal_obstacle_l2v2 = main_grid.AddCellType("wall_l2v6", "interal obstacle wall. l2 v2", '^');
            m_door_tile_left_l2 = main_grid.AddCellType("wall_l2v7", "left door tile wall. l2", '&');
            m_door_tile_right_l2 = main_grid.AddCellType("wall_l2v8", "right door tile wall. l2", '*');
            m_upper_external_l3 = main_grid.AddCellType("wall_l3v1", "upper wall. l3", '9');
            m_lower_external_l3 = main_grid.AddCellType("wall_l3v2", "lower wall. l3", '0');
            m_left_external_l3 = main_grid.AddCellType("wall_l3v3", "left external wall. l3", '-');
            m_right_external_l3 = main_grid.AddCellType("wall_l3v4", "right external  wall. l3", '=');
            m_internal_obstacle_l3v1 = main_grid.AddCellType("wall_l3v5", "interal obstacle wall. l3 v1", '[');
            m_internal_obstacle_l3v2 = main_grid.AddCellType("wall_l3v6", "interal obstacle wall. l3 v2", ']');
            m_door_tile_left_l3 = main_grid.AddCellType("wall_l3v7", "left door tile wall. l3", '.');
            m_door_tile_right_l3 = main_grid.AddCellType("wall_l3v8", "right door tile wall. l3", ';');
            // m_upper_external_l4 = main_grid.AddCellType("wall_l4v1", "upper wall. l4", '(');
            // m_lower_external_l4 = main_grid.AddCellType("wall_l4v2", "lower wall. l4", ')');
            // m_left_external_l4 = main_grid.AddCellType("wall_l4v3", "left external wall. l4", '_');
            // m_right_external_l4 = main_grid.AddCellType("wall_l4v4", "right external  wall. l4", '+');
            // m_internal_obstacle_l4v1 = main_grid.AddCellType("wall_l4v5", "interal obstacle wall. l4 v1", '{');
            // m_internal_obstacle_l4v2 = main_grid.AddCellType("wall_l4v6", "interal obstacle wall. l4 v2", '}');
            // m_door_tile_left_l4 = main_grid.AddCellType("wall_l4v7", "left door tile wall. l4", '~');
            // m_door_tile_right_l4 = main_grid.AddCellType("wall_l4v8", "right door tile wall. l4", ':');

            m_trap_tile = main_grid.AddCellType("wall_trap", "trap tile wall.", 't');
            m_loot_tile = main_grid.AddCellType("wall_loot", "loot tile wall.", 'l');
            m_monster_tile_skeleton = main_grid.AddCellType("wall_skeleton", "monster tile wall. skeleton", 's');
            m_monster_tile_goblin = main_grid.AddCellType("wall_goblin", "monster tile wall. goblin", 'g');

            m_secret_door_top = main_grid.AddCellType("wall_secret_top", "secret tile wall. Top", 'f');
            m_secret_door_bottom = main_grid.AddCellType("wall_secret_bottom", "secret tile wall. Bottom", 'T');
            m_secret_door_left = main_grid.AddCellType("wall_secret_left", "secret tile wall. Left", 'U');
            m_secret_door_right = main_grid.AddCellType("wall_secret_right", "secret tile wall. Right", 'v');


            m_exit_door_l1 = main_grid.AddCellType("exit_l1", "secret exit l1.", 'e');
            m_exit_door_l2 = main_grid.AddCellType("exit_l2", "secret exit l2.", 'u');
            m_exit_door_l3 = main_grid.AddCellType("exit_l3", "secret exit l3.", 'r');
            //m_exit_door_l4 = main_grid.AddCellType("exit_l4", "secret exit l4.", 'R');

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

			auto& player = AddAgent<PlayerAgent>("Player");
			player.SetSymbol('Z').SetLocation(WorldPosition{1,1});
			mPlayer = &player;
            m_player_id = player.GetID();
            m_enemy_id = GetNextAgentId();

			auto& ui = AddAgent<TrashInterface>("Interface");
			ui.SetSymbol(' ').SetLocation(WorldPosition{1,1});

            GenerateLevel();
        }

        ~DungeonWorld() = default;

        /// @brief Generates the DungeonWorld based on the currently selected level from m_level
        void GenerateLevel() {
            LoadLevelData();

			m_generation = std::make_unique<WorldGeneration>(*m_level);
			m_generation->CreateDungeon(m_level_num);
			main_grid.Load(m_generation->GetDungeon());

			SpawnDungeonAgents();
        }

        int GetLevel() {
            return m_level_num;
        }

		void AdvanceLevel() {
			DespawnDungeonAgents();

			if (m_generation) {
				m_generation->ClearLevel();
			}

            ++m_level_num;
            GenerateLevel();

			mLevelJustAdvanced = true;
        }

		void SpawnDungeonAgents() {
			if (!m_generation) return;

			m_spawned_enemy_ids.clear();

			int goblin_num = 1;
			for (const auto &[x, y] : m_generation->GetGoblinSpawns()) {
				auto &agent = AddAgent<PacingAgent>("Goblin " + std::to_string(goblin_num++));
				agent.SetLocation(WorldPosition{x, y});
				m_spawned_enemy_ids.push_back(agent.GetID());
			}

			int skeleton_num = 1;
			for (const auto &[x, y] : m_generation->GetSkeletonSpawns()) {
				auto &agent = AddAgent<PacingAgent>("Skeleton " + std::to_string(skeleton_num++));
				agent.SetLocation(WorldPosition{x, y});
				m_spawned_enemy_ids.push_back(agent.GetID());
			}
		}

		void DespawnDungeonAgents() {
			for (size_t id : m_spawned_enemy_ids) {
				if (AgentBase *agent = TryGetAgent(id)) {
					if (agent->IsAlive()) {
						agent->TakeDamage(agent->GetCurrentHealth());
					}
				}
			}

			RemoveDeadAgents();
			m_spawned_enemy_ids.clear();
		}

        void UpdateWorld() override {
        	if ((m_level_num == 5)) {
        		mRunOver = true;
        		return;
        	}
		}


        /// @brief Grabs user input to determine whether or not to move to the next level.
        /// Debug tool used for updating level progression + progressing dungeon levels (dungeonone, dungeontwo, dungeonthree)
        // void UserInput() {
        //     std::cout << "Success! Continue or quit? ('c'/'q')" << std::endl;
        //     char input;
        //     bool user_checker = true;
        //     while (user_checker) {
        //         std::cin >> input;
        //         if (std::tolower(input) == 'c') {
        //             user_checker = false;
        //         } else if (std::tolower(input) == 'q') {
        //             std::cout << "DungeonGame terminated" << std::endl;
        //             exit(-1);
        //         } else {
        //             std::cout << "Invalid Command!" << std::endl;
        //             continue;
        //         }
        //     }
        //     return;
        // }

		void RunAgents() override {
			TrashInterface* ui = nullptr;

			for (const auto& agent_ptr : agent_set) {
				if (agent_ptr->IsInterface()) {
					ui = static_cast<TrashInterface*>(agent_ptr.get());
					break;
				}
			}

			assert(ui);
			assert(mPlayer);

			size_t action = ui->SelectAction(main_grid);
			int result = DoAction(*mPlayer, action);
			mPlayer->SetActionResult(result);

			ui->SetLocation(mPlayer->GetLocation());

			//Prevent enemies from taking a turn before new level is drawn
			if (mLevelJustAdvanced) {
				mLevelJustAdvanced = false;
				return;
			}

			for (const auto& agent_ptr : agent_set) {
				AgentBase* agent = agent_ptr.get();

				if (agent->IsPlayerAgent()) continue;
				if (agent->IsInterface()) continue;

				size_t action_id = agent->SelectAction(main_grid);
				int result = DoAction(*agent, action_id);
				agent->SetActionResult(result);
			}
		}

        /*
        * @breif This function kills an enemy agent, and rewards the player agent for it.
        * @param enemy - the enemy agent being defeated
        * @param player - the player agent being defeated
        * @note - this function was adapted from the combat system in Group 2's demo code
        */
        void HandleEnemyDefeat(Enemy& enemy, PlayerAgent& player) {
            const std::size_t goldReward = enemy.ClaimGoldDrop();

            std::cout << "Enemy defeated.\n";
            if (goldReward > 0) {
                player.AddGold(goldReward);
                std::cout << player.GetName() << " gains " << goldReward << " gold.\n";
            }
        }

        /*
        * @breif Allow the agents to move around the maze.
        * @param agent - the agent performing an action
        * @param action_id - the id of the action being done
        * @return An integer representing the success state 
        */
        int DoAction(AgentBase &agent, size_t action_id) override {
            // Determine where the agent is trying to move.
            WorldPosition cur_position = agent.GetLocation().AsWorldPosition();

            if (action_id == WorldActions::INTERACT) {
                // Legacy method of handling interactions. Kept here in case below code does not work with both agent groups.
                /*std::array<WorldPosition, 4> neighbors = {
                    cur_position.Up(), cur_position.Down(), cur_position.Left(), cur_position.Right()
                };
                for (const auto &adj: neighbors) {
                    if (main_grid.IsValid(adj) && main_grid[adj] == m_secret_door) {
                        main_grid[adj] = m_floor_id_l1v1;
                        return true;
                    }
                }
                return false;*/

                /////////////////////////////////////////////////////
                // The following code inside this if statement was adapted from the combat system in Group 2's demo code
                bool interacted = false;
                
                for (size_t i = 0; i < GetNumAgents(); ++i) {
                    AgentBase& other = GetAgentByIndex(i);
                    if (&other == &agent) {
                        continue;
                    }
                    if (!other.IsAlive()) {
                        continue;
                    }
                    const WorldPosition other_pos = other.GetLocation().AsWorldPosition();
                    const double dx = std::abs(cur_position.X() - other_pos.X());
                    const double dy = std::abs(cur_position.Y() - other_pos.Y());

                    if (dx <= 1.0 && dy <= 1.0) {
                        interacted = true;
                        if (other.GetID() == m_player_id && HasAgent(m_enemy_id) && agent.GetID() == GetAgent(m_enemy_id).GetID()) {
                            auto& player = dynamic_cast<PlayerAgent&>(other);
                            auto& enemy = dynamic_cast<Enemy&>(agent);
                            
                            const double dealt = DamageCalculator::Calculate(GetAgent(m_enemy_id).GetStats(), GetPlayer()->GetStats());
                            player.TakeDamage(dealt);
                            std::cout << enemy.GetName() << " hits " << player.GetName() << " for " << static_cast<int>(dealt) << " damage.\n";
                            if (!player.IsAlive()) {
                                std::cout << player.GetName() << " has fallen.\n";
                                mRunOver = true;
                                return 1;
                            }
                            const double retaliate = DamageCalculator::Calculate(GetPlayer()->GetStats(), GetAgent(m_enemy_id).GetStats());
                            enemy.TakeDamage(retaliate);
                            std::cout << player.GetName() << " strikes back for " << static_cast<int>(retaliate) << " damage.\n";
                            if (!enemy.IsAlive()) {
                                HandleEnemyDefeat(enemy, player);
                                return 1;
                            }
                            //
                            } else if (other.GetID() == m_enemy_id) {
                                const double dealt = DamageCalculator::Calculate(GetPlayer()->GetStats(), GetAgent(m_enemy_id).GetStats());
                                other.TakeDamage(dealt);
                                std::cout << agent.GetName() << " hits enemy for " << static_cast<int>(dealt) << " damage.\n";
                                if (!other.IsAlive()) {
                                    auto& enemy = dynamic_cast<Enemy&>(other);
                                    auto& player = dynamic_cast<PlayerAgent&>(agent);
                                    HandleEnemyDefeat(enemy, player);
                                    return 1;
                                }
                                const double retaliate = DamageCalculator::Calculate(GetAgent(m_enemy_id).GetStats(), GetPlayer()->GetStats());
                                agent.TakeDamage(retaliate);
                                std::cout << "Enemy strikes back for " << static_cast<int>(retaliate) << " damage.\n";
                                if (!agent.IsAlive()) {
                                    std::cout << agent.GetName() << " has fallen.\n";
                                    mRunOver = true;
                                    return 1;
                                }
                            }
                        }
                    }
                    if (!interacted) {
                        std::cout << "No one nearby to interact with.\n";
                    }
                    return interacted ? 1 : 0;
                    /// The above code inside this if statement was adapted from the combat system in Group 2's demo code
                    /////////////////////////////////////////////////////
                }

            WorldPosition new_position;
            switch (action_id) {
                case REMAIN_STILL: new_position = cur_position;
                    break;
                case MOVE_UP: new_position = cur_position.Up();
                    break;
                case MOVE_DOWN: new_position = cur_position.Down();
                    break;
                case MOVE_LEFT: new_position = cur_position.Left();
                    break;
                case MOVE_RIGHT: new_position = cur_position.Right();
                    break;
                default:
                    break;
            }

            // Don't let the agent move off the world or into a wall.
            if (!main_grid.IsValid(new_position)) { return false; }
            if (main_grid[new_position] == m_wall_id
				|| main_grid[new_position] == m_upper_external_l1
                || main_grid[new_position] == m_lower_external_l1
                || main_grid[new_position] == m_left_external_l1
                || main_grid[new_position] == m_right_external_l1
                || main_grid[new_position] == m_internal_obstacle_l1v1
                || main_grid[new_position] == m_internal_obstacle_l1v2

				|| main_grid[new_position] == m_upper_external_l2
                || main_grid[new_position] == m_lower_external_l2
                || main_grid[new_position] == m_left_external_l2
                || main_grid[new_position] == m_right_external_l2
                || main_grid[new_position] == m_internal_obstacle_l2v1
                || main_grid[new_position] == m_internal_obstacle_l2v2

				|| main_grid[new_position] == m_upper_external_l3
                || main_grid[new_position] == m_lower_external_l3
                || main_grid[new_position] == m_left_external_l3
                || main_grid[new_position] == m_right_external_l3
                || main_grid[new_position] == m_internal_obstacle_l3v1
                || main_grid[new_position] == m_internal_obstacle_l3v2

				// Currently there is no level 4
				// || main_grid[new_position] == m_upper_external_l4
                // || main_grid[new_position] == m_lower_external_l4
                // || main_grid[new_position] == m_left_external_l4
                // || main_grid[new_position] == m_right_external_l4
                // || main_grid[new_position] == m_internal_obstacle_l4v1
                // || main_grid[new_position] == m_internal_obstacle_l4v2
			) { return false; }

            if (main_grid[new_position] == m_loot_tile && agent.GetID() == 0) {
                if (agent.IsPlayerAgent()) {
                    Inventory &inventory = agent.GetInventory();
                    std::string item_str = GetRandomItem();
                    Random rng;
                    size_t item_id = rng.GetValue(MIN_ID, MAX_ID).value();

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
                    } else if (item_str == "Sword +1") {
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
                    } else if (item_str == "Sword +2") {
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
                    } else if (item_str == "Sword +3") {
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
                    } else if (item_str == "Sword +4") {
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
                    } else if (item_str == "Sword +5") {
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
                        bow->SetRange(BASE_RANGE + 3);
                        bow->SetDamage(BASE_DAMAGE + 0.5);
                        bow->SetHitBonus(BASE_BONUS);

                        inventory.AddItem(std::move(bow));
                    } else if (item_str == "Bow +1") {
                        std::unique_ptr<ItemWeaponBow> bow1 = std::make_unique<ItemWeaponBow>(
                            item_id,
                            item_str,
                            "/assets/item/item_bow_1.png",
                            BASE_GOLD + 2,
                            *this
                        );
                        bow1->SetRange(BASE_RANGE + 3);
                        bow1->SetDamage(BASE_DAMAGE + 1.0);
                        bow1->SetHitBonus(BASE_BONUS + 1);

                        inventory.AddItem(std::move(bow1));
                    } else if (item_str == "Bow +2") {
                        std::unique_ptr<ItemWeaponBow> bow2 = std::make_unique<ItemWeaponBow>(
                            item_id,
                            item_str,
                            "/assets/item/item_bow_1.png",
                            BASE_GOLD + 3,
                            *this
                        );
                        bow2->SetRange(BASE_RANGE + 3);
                        bow2->SetDamage(BASE_DAMAGE + 1.5);
                        bow2->SetHitBonus(BASE_BONUS + 2);

                        inventory.AddItem(std::move(bow2));
                    } else if (item_str == "Bow +3") {
                        std::unique_ptr<ItemWeaponBow> bow3 = std::make_unique<ItemWeaponBow>(
                            item_id,
                            item_str,
                            "/assets/item/item_bow_1.png",
                            BASE_GOLD + 4,
                            *this
                        );
                        bow3->SetRange(BASE_RANGE + 3);
                        bow3->SetDamage(BASE_DAMAGE + 2.0);
                        bow3->SetHitBonus(BASE_BONUS + 3);

                        inventory.AddItem(std::move(bow3));
                    } else if (item_str == "Bow +4") {
                        std::unique_ptr<ItemWeaponBow> bow4 = std::make_unique<ItemWeaponBow>(
                            item_id,
                            item_str,
                            "/assets/item/item_bow_1.png",
                            BASE_GOLD + 5,
                            *this
                        );
                        bow4->SetRange(BASE_RANGE + 3);
                        bow4->SetDamage(BASE_DAMAGE + 2.5);
                        bow4->SetHitBonus(BASE_BONUS + 4);

                        inventory.AddItem(std::move(bow4));
                    } else if (item_str == "Bow +5") {
                        std::unique_ptr<ItemWeaponBow> bow5 = std::make_unique<ItemWeaponBow>(
                            item_id,
                            item_str,
                            "/assets/item/item_bow_1.png",
                            BASE_GOLD + 6,
                            *this
                        );
                        bow5->SetRange(BASE_RANGE + 4);
                        bow5->SetDamage(BASE_DAMAGE + 3.0);
                        bow5->SetHitBonus(BASE_BONUS + 5);

                        inventory.AddItem(std::move(bow5));
                    } else if (item_str == "Healing Potion") {
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
                    } else if (item_str == "Defense Potion") {
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
                    } else if (item_str == "Speed Potion") {
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
                    } else if (item_str == "Axe") {
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
                    } else if (item_str == "Pickaxe") {
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
                    } else if (item_str == "Shovel") {
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
			
            if (main_grid[new_position] == m_exit_door_l1 || main_grid[new_position] == m_exit_door_l2 || 
                main_grid[new_position] == m_exit_door_l3 || main_grid[new_position] == m_exit_door_l4) {
                if (agent.IsPlayerAgent()) {
                    // UserInput();
                    AdvanceLevel();
                    new_position = WorldPosition(1, 1); //default player location upon loading into new world
                }
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
        std::string GetRandomItem() {
            cse498::Random rng;
            double item = rng.GetValue(1.0, m_item_pool.GetTotalWeight()).value();
            return m_item_pool.Sample(item).value();
        }
    };
} // End of namespace cse498
