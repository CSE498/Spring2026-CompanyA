/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief A procedurally generated dungeon world using BSP room placement.
 * @author Group 17
 * @note Status: PROPOSAL (PLACEHOLDER WORLD)
 * Made via reference to Claude Sonnet 4.6
 */

#pragma once

#include "../../../../group_specific_content/Group-15/WorldGeneration.hpp"
#include "../../../core/WorldBase.hpp"

namespace cse498
{

    class DungeonWorld : public WorldBase
    {
    protected:
        enum ActionType
        {
            REMAIN_STILL = 0,
            MOVE_UP,
            MOVE_DOWN,
            MOVE_LEFT,
            MOVE_RIGHT
        };

        size_t wall_id; ///< '#' — impassable wall
        size_t floor_id; ///< ' ' — open floor (space character)
        size_t dot_id; ///< '.' — open floor (dot character)
        size_t door_id; ///< '+' — door, treated as walkable

        LegacyWorldGeneration mWorldGen; ///< BSP dungeon generator

        /// Provide agents with movement actions
        void ConfigAgent(AgentBase &agent) override
        {
            agent.AddAction("up", MOVE_UP);
            agent.AddAction("down", MOVE_DOWN);
            agent.AddAction("left", MOVE_LEFT);
            agent.AddAction("right", MOVE_RIGHT);
        }

    public:
        DungeonWorld()
        {
            // Register cell types matching room file characters
            wall_id = main_grid.AddCellType("wall", "Impassable wall.", '#');
            floor_id = main_grid.AddCellType("floor", "Open floor.", ' ');
            dot_id = main_grid.AddCellType("dot", "Open floor (dot).", '.');
            door_id = main_grid.AddCellType("door", "Door, walkable.", '+');

            // Generate the dungeon
            mWorldGen.CreateDungeon();

            // Load the generated grid into WorldGrid
            main_grid.Load(mWorldGen.GetDungeon());
        }

        ~DungeonWorld() = default;

        // -- Accessors --
        [[nodiscard]] size_t GetWallID() const { return wall_id; }
        [[nodiscard]] size_t GetFloorID() const { return floor_id; }
        [[nodiscard]] size_t GetDotID() const { return dot_id; }
        [[nodiscard]] size_t GetDoorID() const { return door_id; }

        /// Regenerate the dungeon — call when entering a new level
        void Regenerate()
        {
            mWorldGen = LegacyWorldGeneration();
            mWorldGen.CreateDungeon();
            main_grid.Load(mWorldGen.GetDungeon());
        }

        /// Check if a position is walkable (anything that isn't a wall)
        [[nodiscard]] bool IsWalkable(const WorldPosition &pos) const
        {
            if (!main_grid.IsValid(pos))
                return false;
            return main_grid[pos] != wall_id;
        }

        /// Allow agents to move around the dungeon
        int DoAction(AgentBase &agent, size_t action_id) override
        {
            WorldPosition cur_pos = agent.GetLocation().AsWorldPosition();
            WorldPosition new_pos;

            switch (action_id)
            {
            case REMAIN_STILL:
                new_pos = cur_pos;
                break;
            case MOVE_UP:
                new_pos = cur_pos.Up();
                break;
            case MOVE_DOWN:
                new_pos = cur_pos.Down();
                break;
            case MOVE_LEFT:
                new_pos = cur_pos.Left();
                break;
            case MOVE_RIGHT:
                new_pos = cur_pos.Right();
                break;
            default:
                new_pos = cur_pos;
                break;
            }

            if (!IsWalkable(new_pos))
                return false;

            agent.SetLocation(new_pos);
            return true;
        }
    };

} // namespace cse498