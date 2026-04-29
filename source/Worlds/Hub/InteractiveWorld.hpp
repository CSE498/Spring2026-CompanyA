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
#include "../../Agents/AI/LearningExplorerAgent.hpp"
#include "../../Agents/Classic/ResourceManagementAgent.hpp"
#include "../../Agents/PacingAgent.hpp"
#include "../../core/WorldBase.hpp"
#include "Building.hpp"
#include "InteractiveWorldInventory.hpp"
#include "ResourceBank.hpp"
#include "ResourceProducer.hpp"
#include "ResourceSpawn.hpp"
#include "TownHall.hpp"

namespace cse498 {
/**
 * @class InteractiveWorld
 * @brief Overworld resource-logistics world used by the GUI demo.
 *
 * InteractiveWorld owns the terrain grid, player, resource buildings,
 * resource producers, and shared resource inventory. GUI setup code adds the
 * current town hall, spawns, buildings, fetchers, and resource-management NPC.
 */
class InteractiveWorld : public WorldBase {
protected:
    // World Inventory
    std::shared_ptr<InteractiveWorldInventory> m_inventory = std::make_shared<InteractiveWorldInventory>();

    enum ActionType { REMAIN_STILL = 0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, INTERACT, QUIT };

    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id; ///< Easy access to wall CellType ID.
    size_t building_id; ///< Non-walkable object placeholder cell.

    size_t ow_wall_left_id; ///< Left border wall cell type.
    size_t ow_wall_right_id; ///< Right border wall cell type.
    size_t ow_wall_top_id; ///< Top border wall cell type.
    size_t ow_wall_bottom_id; ///< Bottom border wall cell type.

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
    /// Resource producers currently active in the scene.
    std::vector<std::shared_ptr<ResourceProducer>> m_producers{};

    /**
     * @brief Build a position-to-agent lookup for adjacent interactions.
     * @param ignoredAgent Agent to omit from the map, typically the initiator.
     * @return Map of occupied world positions to agents.
     */
    [[nodiscard]] std::unordered_map<WorldPosition, AgentBase*> BuildAgentPositionMap(const AgentBase* ignoredAgent) {
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

    /**
     * @brief Try to interact with a neighboring agent.
     * @param position Initiator position.
     * @param initiator Agent initiating the interaction.
     * @return true if a neighboring agent handled the interaction.
     */
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

    /**
     * @brief Place a world object on a non-walkable object tile.
     * @tparam T Agent-like object type with SetLocation().
     * @param object Object to place.
     * @param position Grid position to occupy.
     */
    template<typename T>
    void PlaceWorldObject(T& object, WorldPosition position) {
        object.SetLocation(Location(position));
        main_grid[position] = building_id;
    }

    /**
     * @brief Save core InteractiveWorld state to a JSON file.
     * @param filename Destination file.
     * @return true if the file was written.
     */
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
     * @brief Update text-demo world logic.
     *
     * The GUI path updates producers and agents from Game.cpp so it can control
     * frame timing and autonomous movement. This override remains for the older
     * non-GUI run loop.
     */
    void UpdateWorld() override {
        for (const auto& producer: m_producers) {
            producer->Update();
        }

        PrintInventory(); // Shows inventory status, for demo/simple game purposes
    }

    /**
     * @brief Print world inventory totals for the text demo.
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
     * @brief Construct the base InteractiveWorld terrain and player.
     *
     * Resource lanes, buildings, producers, and the resource manager are added
     * by demo setup code so GUI and text demos can configure their own layouts.
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
                "CUUUUUUUUUUUUUUUC", "L...f.f.m....f..R", "L...f...m....f..R", "L.......m...ff..R", "L.f...f.m.......R",
                "L...f...m.f.....R", "LmmmmmmmmmmmmmmmR", "L.......m..ff...R", "L..f..f.m.......R", "L....f..m.......R",
                "L.f.....m.....f.R", "L....f..m.......R", "CBBBBBBBBBBBBBBBC"});

        auto& player = AddAgent<PlayerAgent>("Player");
        player.SetSymbol('Z').SetLocation(WorldPosition{1, 1});
        mPlayer = &player;
    }

    /// @brief Default destructor.
    ~InteractiveWorld() = default;

    /**
     * @brief Get all mutable building agents in the world.
     * @return Vector of Building pointers.
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

    /**
     * @brief Get all building agents in the world.
     * @return Vector of const Building pointers.
     */
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
    /// @return Read-only world resource inventory.
    const InteractiveWorldInventory& GetInventory() const { return *m_inventory; }
    /// @return Shared world inventory pointer used by town hall and manager agents.
    std::shared_ptr<InteractiveWorldInventory> GetInventoryPtr() { return m_inventory; }
    /// @return Active resource producers. GUI code ticks these directly.
    [[nodiscard]] const std::vector<std::shared_ptr<ResourceProducer>>& GetProducers() const { return m_producers; }
    /**
     * @brief Have an agent perform an InteractiveWorld action.
     * @param agent Agent to perform action on.
     * @param action_id ActionType id to perform.
     * @return Non-zero on success, zero on blocked movement.
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
        if (cell == wall_id || cell == building_id || cell == ow_wall_left_id || cell == ow_wall_right_id ||
            cell == ow_wall_top_id || cell == ow_wall_bottom_id) {
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
     * @brief Add a producer to the active producer list.
     * @param producer Producer to add.
     */
    void AddProducer(std::shared_ptr<ResourceProducer> producer) { m_producers.push_back(producer); }

    /**
     * @brief Place a building in the world.
     * @param building Building to place.
     * @param position Grid position to occupy.
     */
    void AddBuilding(Building& building, WorldPosition position) { PlaceWorldObject(building, position); }

    /**
     * @brief Place a resource spawn in the world.
     * @param spawn Resource spawn to place.
     * @param position Grid position to occupy.
     */
    void AddResourceSpawn(ResourceSpawn& spawn, WorldPosition position) { PlaceWorldObject(spawn, position); }

    /**
     * @brief Place a resource bank in the world.
     * @param bank Resource bank to place.
     * @param position Grid position to occupy.
     */
    void AddResourceBank(ResourceBank& bank, WorldPosition position) { PlaceWorldObject(bank, position); }

    /**
     * @brief Place the town hall in the world.
     * @param th Town hall to place.
     * @param position Grid position to occupy.
     */
    void AddTownHall(TownHall& th, WorldPosition position) { PlaceWorldObject(th, position); }

    /**
     * @brief Remove a building's blocking tile from the grid.
     * @param building Building whose tile should be cleared.
     */
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
    LearningExplorerAgent& AddLearningExplorerAgent(size_t x, size_t y) {
        LearningExplorerAgent& agent = AddAgent<LearningExplorerAgent>("Explorer");
        agent.SetLocation(WorldPosition(x, y));
        return agent;
    }

    /**
     * @brief Add a simple pacing agent to the overworld.
     * @param sprite_name Sprite key used by older render paths.
     * @param x Grid-cell X coordinate.
     * @param y Grid-cell Y coordinate.
     * @param horizontal true for horizontal pacing, false for vertical pacing.
     */
    void AddPacingAgent(const std::string& sprite_name, size_t x, size_t y, bool horizontal = true) {
        mAgentSpriteName = sprite_name;
        PacingAgent& agent = AddAgent<PacingAgent>("Skeleton");
        if (horizontal) {
            agent.SetHorizontal();
        } else {
            agent.SetVertical();
        }
        agent.SetLocation(WorldPosition(x, y));
    }

    /// @return Sprite key assigned by AddPacingAgent().
    [[nodiscard]] const std::string& GetAgentSpriteName() const { return mAgentSpriteName; }
};
}; // namespace cse498
