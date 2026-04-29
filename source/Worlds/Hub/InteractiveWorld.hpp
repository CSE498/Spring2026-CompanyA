/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents the Interactive World module
 **/

#pragma once
#include <algorithm>
#include <array>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
#include "../../../third-party/json/json.hpp"
#include "../../Agents/Classic/ResourceManagementAgent.hpp"
#include "../../core/WorldBase.hpp"
#include "Building.hpp"
#include "InteractiveWorldInventory.hpp"
#include "ResourceBank.hpp"
#include "ResourceProducer.hpp"
#include "ResourceSpawn.hpp"
#include "TownHall.hpp"
#include "../../Agents/PacingAgent.hpp"
#include "../../Agents/AI/LearningExplorerAgent.hpp"

namespace cse498 {
/**
 * World object for the interactive world
 */
class InteractiveWorld : public WorldBase {
protected:
    // World Inventory
    std::shared_ptr<InteractiveWorldInventory> m_inventory = std::make_shared<InteractiveWorldInventory>();

    enum ActionType { REMAIN_STILL = 0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, INTERACT, QUIT };

    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id; ///< Easy access to wall CellType ID.
    size_t building_id;

    // GUI requirements
    size_t ow_wall_left_id;
    size_t ow_wall_right_id;
    size_t ow_wall_top_id;
    size_t ow_wall_bottom_id;

    std::string mAgentSpriteName; ///< Sprite name for rendering agents

    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase& agent) override {
        agent.AddAction("up", MOVE_UP);
        agent.AddAction("down", MOVE_DOWN);
        agent.AddAction("left", MOVE_LEFT);
        agent.AddAction("right", MOVE_RIGHT);
        agent.AddAction("interact", INTERACT);
        agent.AddAction("quit", QUIT);
    }

private:
    // ResourceProducers in the scene
    std::vector<std::shared_ptr<ResourceProducer>> m_producers{};

    [[nodiscard]] std::unordered_map<WorldPosition, AgentBase*> BuildAgentPositionMap(
        const AgentBase* ignoredAgent) {
        std::unordered_map<WorldPosition, AgentBase*> agentsByPosition;
        for (size_t i = 0; i < GetNumAgents(); ++i) {
            AgentBase& candidate = GetAgentByIndex(i);
            if (&candidate == ignoredAgent || !candidate.GetLocation().IsPosition()) {
                continue;
            }

            agentsByPosition.emplace(candidate.GetLocation().AsWorldPosition(), &candidate);
        }
        return agentsByPosition;
    }

    bool TryInteractAdjacent(const WorldPosition& position, const AgentBase* initiator) {
        const std::array<WorldPosition, 4> priorities = {
                position.Right(),
                position.Up(),
                position.Down(),
                position.Left(),
        };

        const auto agentsByPosition = BuildAgentPositionMap(initiator);
        for (const WorldPosition& target: priorities) {
            const auto it = agentsByPosition.find(target);
            if (it != agentsByPosition.end() && it->second->Interact()) {
                return true;
            }
        }

        return false;
    }

    template<typename T>
    void PlaceWorldObject(T& object, WorldPosition position) {
        object.SetLocation(Location(position));
        main_grid[position] = building_id;
    }

    bool SaveToFile(const std::string& filename) const {
        nlohmann::json j;

        const auto& inv = GetInventory();
        j["inventory"]["wood"] = inv.GetAmount(ItemType::Wood);
        j["inventory"]["stone"] = inv.GetAmount(ItemType::Stone);
        j["inventory"]["metal"] = inv.GetAmount(ItemType::Metal);

        j["buildings"] = nlohmann::json::array();
        for (const auto& buildingPtr: GetBuildings()) {
            if (buildingPtr == nullptr) {
                continue;
            }

            nlohmann::json buildingJson;
            buildingJson["name"] = buildingPtr->GetName();
            buildingJson["level"] = buildingPtr->GetCurrentLevel();
            j["buildings"].push_back(buildingJson);
        }

        for (size_t i = 0; i < GetNumAgents(); ++i) {
            const auto* manager = dynamic_cast<const ResourceManagementAgent*>(&GetAgentByIndex(i));
            if (manager == nullptr) {
                continue;
            }

            j["resource_manager"]["name"] = manager->GetName();
            j["resource_manager"]["gold"] = manager->GetGold();
            j["resource_manager"]["lanes"] = nlohmann::json::array();
            for (std::size_t laneIndex = 0; laneIndex < manager->GetHireableLaneCount(); ++laneIndex) {
                nlohmann::json laneJson;
                laneJson["label"] = manager->GetHireableLaneLabel(laneIndex);
                laneJson["unlocked"] = manager->IsLaneUnlocked(laneIndex);
                j["resource_manager"]["lanes"].push_back(laneJson);
            }

            break;
        }

        std::ofstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        file << j.dump(4);
        return true;
    }

    /**
     * Update world logic
     */
    void UpdateWorld() override {
        for (const auto& producer: m_producers) {
            producer->Update();
        }

        PrintInventory(); // Shows inventory status, for demo/simple game purposes
    }

    /**
     * Print the world inventory
     */
    void PrintInventory() {
        std::ostringstream output;
        output << m_inventory->GetAmount(ItemType::Wood) << ' ' << ItemTypeToString(ItemType::Wood) << " | "
               << m_inventory->GetAmount(ItemType::Stone) << ' ' << ItemTypeToString(ItemType::Stone) << " | "
               << m_inventory->GetAmount(ItemType::Metal) << ' ' << ItemTypeToString(ItemType::Metal);

        std::cout << output.str() << std::endl;
    }

    /**
     * Is a building at a position
     * @param position position to test
     * @return if a building is at that position
     */
    bool IsBuildingAt(const WorldPosition& position) const {
        for (size_t i = 0; i < GetNumAgents(); i++) {
            const auto& agent = GetAgentByIndex(i);
            if (const auto* b = dynamic_cast<const Building*>(&agent)) {
                if (b->GetLocation().IsPosition() && b->GetLocation().AsWorldPosition() == position) {
                    return true;
                }
            }
        }
        return false;
    }


public:
    /**
     * Constructor
     */
    InteractiveWorld() {
        // Overworld grass variants
        floor_id = main_grid.AddCellType("ow_grass", "Plain grass.", '.');
        main_grid.AddCellType("ow_grass_flowers", "Flowery grass.", 'f');
        main_grid.AddCellType("ow_grass_bones", "Bone-strewn grass.", 'b');
        main_grid.AddCellType("ow_grass_mud", "Muddy grass.", 'm');
        main_grid.AddCellType("ow_grass_rock", "Rocky grass.", 'r');

        // Structure
        main_grid.AddCellType("ow_entrance", "Entrance to the dungeon.", 'E');

        // Border walls
        ow_wall_left_id = main_grid.AddCellType("ow_wall_left", "Left wall.", 'L');
        ow_wall_right_id = main_grid.AddCellType("ow_wall_right", "Right wall.", 'R');
        ow_wall_top_id = main_grid.AddCellType("ow_wall_top", "Top wall.", 'U');
        ow_wall_bottom_id = main_grid.AddCellType("ow_wall_bottom", "Bottom wall.", 'B');
        wall_id = main_grid.AddCellType("ow_wall_corner", "Corner wall.", 'C');

        // Building tile
        building_id = main_grid.AddCellType("ow_building", "An impassable building.", 'X');

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

        auto& player = AddAgent<PlayerAgent>("Player");
        player.SetSymbol('Z').SetLocation(WorldPosition{1, 1});
        mPlayer = &player;
    }

    /**
     * Destructor
     */
    ~InteractiveWorld() = default;

    /**
     * Get all buildings in the world
     * @return vector of Building pointers
     */
    std::vector<Building*> GetBuildings() {
        std::vector<Building*> buildings;
        for (size_t i = 0; i < GetNumAgents(); i++) {
            if (auto* b = dynamic_cast<Building*>(&GetAgentByIndex(i))) {
                buildings.push_back(b);
            }
        }
        return buildings;
    }

    std::vector<const Building*> GetBuildings() const {
        std::vector<const Building*> buildings;
        for (size_t i = 0; i < GetNumAgents(); i++) {
            if (const auto* b = dynamic_cast<const Building*>(&GetAgentByIndex(i))) {
                buildings.push_back(b);
            }
        }
        return buildings;
    }

    /**
     * Get the world inventory object
     * @return reference to world inventory
     */
    InteractiveWorldInventory& GetInventory() { return *m_inventory; }
    const InteractiveWorldInventory& GetInventory() const { return *m_inventory; }
    std::shared_ptr<InteractiveWorldInventory> GetInventoryPtr() { return m_inventory; }
    [[nodiscard]] const std::vector<std::shared_ptr<ResourceProducer>>& GetProducers() const { return m_producers; }
    /**
     * Have agent perform an action
     * @param agent Agent to perform action on
     * @param action_id type of action to perform
     */
    int DoAction(AgentBase& agent, size_t action_id) override {
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
                if (agent.GetLocation().IsPosition()) {
                    return TryInteractAdjacent(cur_position, &agent);
                }
                return true;
            case QUIT:
                if (SaveToFile("interactive_world_save.json")) {
                    std::cout << "\nGame saved to interactive_world_save.json\n";
                } else {
                    std::cout << "\nFailed to save game.\n";
                }
                mRunOver = true;
                return true;
        }

        // Don't let the agent move off the world or into a non-walkable tile.
        if (!main_grid.IsValid(new_position)) {
            return false;
        }
        size_t cell = main_grid[new_position];
        if (cell == wall_id || cell == building_id
            || cell == ow_wall_left_id
            || cell == ow_wall_right_id
            || cell == ow_wall_top_id
            || cell == ow_wall_bottom_id) {
            return false;
            }

        // Open NPC UI for interface-controlled agents only.
        if (agent.IsInterface()) {
            // ForEachAdjacentNPC(neighbors, [](NPC &npc) { npc.Interact(); });
        }

        // Set the agent to its new position.
        agent.SetLocation(new_position);

        return true;
    }

    /**
     * Add producer to the world
     * @param producer producer to add
     */
    void AddProducer(std::shared_ptr<ResourceProducer> producer) { m_producers.push_back(producer); }

    /**
     * Add building to world
     * @param building building to add
     * @param position where to add in world
     */
    void AddBuilding(Building& building, WorldPosition position) {
        PlaceWorldObject(building, position);
    }

    /**
     * Add resource spawn to world
     * @param spawn resource spawn to add
     * @param position where to add spawn
     */
    void AddResourceSpawn(ResourceSpawn& spawn, WorldPosition position) {
        PlaceWorldObject(spawn, position);
    }

    /**
     * Add resource bank to world
     * @param bank resource bank to add
     * @param position where to add bank
     */
    void AddResourceBank(ResourceBank& bank, WorldPosition position) {
        PlaceWorldObject(bank, position);
    }

    /**
     * Add Town hall to the world
     * @param th town hall
     * @param position where to add spawn
     */
    void AddTownHall(TownHall& th, WorldPosition position) {
        PlaceWorldObject(th, position);
    }

    void RemoveBuilding(Building& building) {
        if (building.GetLocation().IsPosition()) {
            main_grid[building.GetLocation().AsWorldPosition()] = floor_id; // restore tile
        }
    }

    
    /**
     * @brief Spawn a Group 17 @ref LearningExplorerAgent into the overworld.
     *
     * @details This convenience spawner mirrors @ref AddPacingAgent so @c Game.cpp
     *          can drop the AI agent into the world with one line. The agent is
     *          registered under the name @c "Explorer"; @c Game::RenderOverworld
     *          dispatches on that name to pick the goblin sprite, leaving the
     *          existing skeleton-sprite path for @ref PacingAgent untouched.
     *
     * @param x Grid-cell X coordinate of the spawn tile.
     * @param y Grid-cell Y coordinate of the spawn tile.
     * @return Reference to the newly registered agent, for chained configuration.
     */
    LearningExplorerAgent & AddLearningExplorerAgent(size_t x, size_t y) {
        LearningExplorerAgent & agent = AddAgent<LearningExplorerAgent>("Explorer");
        agent.SetLocation(WorldPosition(x, y));
        return agent;
    }

    /// Add a pacing agent to the overworld at the given position
    void AddPacingAgent(const std::string & sprite_name, size_t x, size_t y, bool horizontal = true) {
        mAgentSpriteName = sprite_name;
        PacingAgent & agent = AddAgent<PacingAgent>("Skeleton");
        if (horizontal) {
        agent.SetHorizontal();
        }
        else {
        agent.SetVertical();
        }
        agent.SetLocation(WorldPosition(x, y));
    }

    [[nodiscard]] const std::string & GetAgentSpriteName() const { return mAgentSpriteName; }
};
}; // namespace cse498
