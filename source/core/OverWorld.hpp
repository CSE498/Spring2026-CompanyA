/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief A flat interactive overworld with a dungeon entrance.
 * @author: Group 17
 * @note Status: PROPOSAL
 */

#pragma once

#include <cassert>

#include "../core/WorldBase.hpp"

namespace cse498 {

  class OverWorld : public WorldBase {
  protected:
    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT };

    // Grass variants
    size_t grass_id;         ///< Plain grass
    size_t grass_flowers_id; ///< Flowery grass (near border)
    size_t grass_bones_id;   ///< Bone grass (near dungeon entrance)
    size_t grass_mud_id;     ///< Muddy grass (bottom zone)
    size_t grass_rock_id;    ///< Rocky grass (top zone)

    // Structure
    size_t entrance_id;      ///< Dungeon entrance

    // Border walls
    size_t wall_left_id;     ///< Left wall   'L'
    size_t wall_right_id;    ///< Right wall  'R'
    size_t wall_top_id;      ///< Top wall    'U'
    size_t wall_bottom_id;   ///< Bottom wall 'B'
    size_t wall_corner_id;   ///< Corner      'C'

    /// Provide agents with movement actions
    void ConfigAgent(AgentBase & agent) override {
      agent.AddAction("up",    MOVE_UP);
      agent.AddAction("down",  MOVE_DOWN);
      agent.AddAction("left",  MOVE_LEFT);
      agent.AddAction("right", MOVE_RIGHT);
    }

  public:
    OverWorld() {
      // Register grass variants
      grass_id         = main_grid.AddCellType("grass",         "Plain grass.",          '.');
      grass_flowers_id = main_grid.AddCellType("grass_flowers", "Flowery grass.",        'f');
      grass_bones_id   = main_grid.AddCellType("grass_bones",   "Bone-strewn grass.",    'b');
      grass_mud_id     = main_grid.AddCellType("grass_mud",     "Muddy grass.",          'm');
      grass_rock_id    = main_grid.AddCellType("grass_rock",    "Rocky grass.",          'r');

      // Register structure tiles
      entrance_id = main_grid.AddCellType("entrance", "Entrance to the dungeon", 'E');

      // Register border walls
      wall_left_id   = main_grid.AddCellType("wall_left",   "Left wall.",   'L');
      wall_right_id  = main_grid.AddCellType("wall_right",  "Right wall.",  'R');
      wall_top_id    = main_grid.AddCellType("wall_top",    "Top wall.",    'U');
      wall_bottom_id = main_grid.AddCellType("wall_bottom", "Bottom wall.", 'B');
      wall_corner_id = main_grid.AddCellType("wall_corner", "Corner wall.", 'C');

      // layout
      // Corners:        C
      // Border walls:   U (top), B (bottom), L (left), R (right)
      // Plain grass:    .
      // Flowers:        f  (near borders — peaceful outer ring)
      // Bones:          b  (around dungeon entrance — ominous zone)
      // Mud:            m  (bottom section — swampy feel)
      // Rock:           r  (top section — rugged feel)
      // Entrance:       E  (center-ish)
      main_grid.Load(std::vector<std::string>{
        "CUUUUUUUUUUUUUUUUUUUUUUUC",
        "LrrrrrrrrrrrrrrrrrrrrrrrR",
        "LrrrrrrrrrrrrrrrrrrrrrrrR",
        "Lrr...................rrR",
        "Lrr.fffffffffffffff..rrrR",
        "Lrr.f...............frrrR",
        "Lrr.f...r.......r...frrrR",
        "Lrr.f...............frrrR",
        "Lrr.fffffffffffffff..rrrR",
        "Lrr...................rrR",
        "Lrr...................rrR",
        "Lrr...................rrR",
        "Lrr.......bbb.........rrR",
        "Lrr......bbfbb........rrR",
        "Lrr.......bbb.........rrR",
        "Lrr...................rrR",
        "LmmmmmmmmmmmmmmmmmmmmmmrR",
        "LmmmmmmmmmmmmmmmmmmmmmmrR",
        "Lmm...................mmR",
        "Lmm...................mmR",
        "Lmm....f.......f......mmR",
        "Lmm...................mmR",
        "LmmmmmmmmmmmmmmmmmmmmmmrR",
        "LmmmmmmmmmmmmmmmmmmmmmmrR",
        "CBBBBBBBBBBBBBBBBBBBBBBBC"
      });
    }

    ~OverWorld() = default;

    // -- Accessors --
    [[nodiscard]] size_t GetGrassID()        const { return grass_id; }
    [[nodiscard]] size_t GetGrassFlowersID() const { return grass_flowers_id; }
    [[nodiscard]] size_t GetGrassBonesID()   const { return grass_bones_id; }
    [[nodiscard]] size_t GetGrassMudID()     const { return grass_mud_id; }
    [[nodiscard]] size_t GetGrassRockID()    const { return grass_rock_id; }
    [[nodiscard]] size_t GetEntranceID()     const { return entrance_id; }

    /// Check if a position is the dungeon entrance
    [[nodiscard]] bool IsEntrance(const WorldPosition & pos) const {
      if (!main_grid.IsValid(pos)) return false;
      return main_grid[pos] == entrance_id;
    }

    /// Check if a position is walkable (not a wall or border)
    [[nodiscard]] bool IsWalkable(const WorldPosition & pos) const {
      if (!main_grid.IsValid(pos)) return false;
      size_t cell = main_grid[pos];
      return cell != wall_left_id
          && cell != wall_right_id
          && cell != wall_top_id
          && cell != wall_bottom_id
          && cell != wall_corner_id;
    }

    /// Allow agents to move around the overworld
    int DoAction(AgentBase & agent, size_t action_id) override {
      WorldPosition cur_pos = agent.GetLocation().AsWorldPosition();
      WorldPosition new_pos;

      switch (action_id) {
        case REMAIN_STILL: new_pos = cur_pos;         break;
        case MOVE_UP:      new_pos = cur_pos.Up();    break;
        case MOVE_DOWN:    new_pos = cur_pos.Down();  break;
        case MOVE_LEFT:    new_pos = cur_pos.Left();  break;
        case MOVE_RIGHT:   new_pos = cur_pos.Right(); break;
        default:           new_pos = cur_pos;         break;
      }

      if (!IsWalkable(new_pos)) return false;

      agent.SetLocation(new_pos);
      return true;
    }
  };

} // namespace cse498