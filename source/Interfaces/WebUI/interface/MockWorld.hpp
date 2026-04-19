#pragma once

#include "../../../core/WorldBase.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstdlib>
#include <memory>

namespace cse498 {

/// @class MockWorld
/// @brief A mock world implementation for testing and demonstrating interfaces.
/// @details This class provides a simple grid-based world with predefined cell
/// types and agents for testing WebUI interfaces.
class MockWorld : public WorldBase {
public:
    /// @enum ActionType
    /// @brief Enumeration of possible agent actions.
    enum ActionType { REMAIN_STILL = 0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, INTERACT, QUIT };

protected:
    /// @brief Array of floor cell type IDs for easy access.
    std::array<size_t, 3> mFloorIds;

    /// @brief Array of impassable cell type IDs for easy access.
    std::array<size_t, 5> mImpassableIds;

    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase& agent) override {
        agent.AddAction("up", MOVE_UP);
        agent.AddAction("down", MOVE_DOWN);
        agent.AddAction("left", MOVE_LEFT);
        agent.AddAction("right", MOVE_RIGHT);
        agent.AddAction("interact", INTERACT);
        agent.AddAction("quit", QUIT);
    }

public:
    /// @brief Constructs a MockWorld with predefined grid layout.
    MockWorld() {
        mFloorIds[0] = main_grid.AddCellType("floor", "assets/world/forest/floor_tiles/tile_grass_1.png", ' ');
        mFloorIds[1] = main_grid.AddCellType("floor-variant1", "assets/world/forest/floor_tiles/tile_grass_5.png", 'v');
        mFloorIds[2] = main_grid.AddCellType("floor-variant2", "assets/world/forest/floor_tiles/tile_grass_2.png", 't');

        // clang-format off
        mImpassableIds[0] = main_grid.AddCellType("floor-obstacle", "assets/world/forest/floor_tiles/tile_grass_3.png", '$');
        mImpassableIds[1] = main_grid.AddCellType("top-wall", "assets/world/forest/walls/external/border_top_forest.png", '^');
        mImpassableIds[2] = main_grid.AddCellType("left-wall", "assets/world/forest/walls/external/border_left_forest.png", '<');
        mImpassableIds[3] = main_grid.AddCellType("right-wall", "assets/world/forest/walls/external/border_right_forest.png", '>');
        mImpassableIds[4] = main_grid.AddCellType("bottom-wall", "assets/world/forest/walls/external/border_bottom_forest.png", '&');
        // clang-format on

        // clang-format off
        main_grid.Load(std::vector<std::string>{
            "^^^^^^^^^^^^^^^^^^",
            "<        t       >",
            "<  vv         $  >",
            "<                >",
            "<  t   $         >",
            "<          t     >",
            "<  vv            >",
            "<  vv        $   >",
            "< $   t          >",
            "<        vvvv    >",
            "&&&&&&&&&&&&&&&&&&"
        });
        // clang-format on

        auto player = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this);
        AddAgent(std::move(player));
        mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
        mPlayer->SetSymbol('@');
        mPlayer->SetLocation(WorldPosition{1, 1});
    }

    ~MockWorld() = default;

    /// @brief Executes an action for the given agent.
    /// @param agent The agent performing the action.
    /// @param action_id The ID of the action to perform.
    /// @return True if the action was successful, false otherwise.
    virtual int DoAction(AgentBase& agent, size_t action_id) override {
        // Determine where the agent is trying to move.
        WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
        WorldPosition new_position;
        switch (action_id) {
            case REMAIN_STILL:
                new_position = cur_position;
                break;
            case MOVE_UP:
                new_position = cur_position.Up();
                break;
            case MOVE_DOWN:
                new_position = cur_position.Down();
                break;
            case MOVE_LEFT:
                new_position = cur_position.Left();
                break;
            case MOVE_RIGHT:
                new_position = cur_position.Right();
                break;
            case INTERACT:
                return true;
            case QUIT:
                return true;
        }

        // Don't let the agent move off the world or into a wall.
        if (!main_grid.IsValid(new_position)) {
            return false;
        }
        if (std::ranges::contains(mImpassableIds, main_grid[new_position])) {
            return false;
        }

        // Set the agent to its new position.
        agent.SetLocation(new_position);
        return true;
    }
};

} // namespace cse498
