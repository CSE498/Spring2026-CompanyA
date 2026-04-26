/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents the Interactive World module
 * @note Status: PROPOSAL
 **/

#pragma once
#include <algorithm>
#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "../../core/WorldBase.hpp"
#include "Building.hpp"
#include "InteractiveWorldInventory.hpp"
#include "ResourceBank.hpp"
#include "ResourceProducer.hpp"
#include "ResourceSpawn.hpp"
#include "TownHall.hpp"

namespace cse498 {
/**
 * World object for the interactive world
 */
class InteractiveWorld : public WorldBase {
protected:
    // World Inventory
    std::shared_ptr<InteractiveWorldInventory> m_inventory = std::make_shared<InteractiveWorldInventory>();

    enum ActionType { REMAIN_STILL = 0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, INTERACT };

    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id; ///< Easy access to wall CellType ID.
    size_t building_id;

    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase& agent) override {
        agent.AddAction("up", MOVE_UP);
        agent.AddAction("down", MOVE_DOWN);
        agent.AddAction("left", MOVE_LEFT);
        agent.AddAction("right", MOVE_RIGHT);
        agent.AddAction("interact", INTERACT);
    }

private:
    // ResourceProducers in the scene
    std::vector<std::shared_ptr<ResourceProducer>> m_producers{};

    bool TryInteractAdjacent(const WorldPosition& position, const AgentBase* initiator) {
        const std::array<WorldPosition, 4> priorities = {
                position.Right(),
                position.Up(),
                position.Down(),
                position.Left(),
        };

        for (const WorldPosition& target: priorities) {
            for (size_t i = 0; i < GetNumAgents(); ++i) {
                AgentBase& candidate = GetAgentByIndex(i);
                if (&candidate == initiator || !candidate.GetLocation().IsPosition()) {
                    continue;
                }

                if (candidate.GetLocation().AsWorldPosition() != target) {
                    continue;
                }

                if (candidate.Interact()) {
                    return true;
                }
            }
        }

        return false;
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
        floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
        wall_id = main_grid.AddCellType("wall", "Impenetrable wall.", '#');
        building_id = main_grid.AddCellType("building", "An impassable building.", 'B');

        main_grid.Load(std::vector<std::string>{
                "#######################", "#                     #", "#                     #",
                "#                     #", "#                     #", "#                     #",
                "#                     #", "#                     #", "#                     #",
                "#                     #", "#######################"});
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
                if (agent.IsInterface() && agent.GetLocation().IsPosition()) {
                    return TryInteractAdjacent(cur_position, &agent);
                }
                return true;
        }

        // Don't let the agent move off the world or into a non-walkable tile.
        if (!main_grid.IsWalkable(new_position)) {
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
        building.SetLocation(Location(position));
        main_grid[position] = building_id;
    }

    /**
     * Add resource spawn to world
     * @param spawn resource spawn to add
     * @param position where to add spawn
     */
    void AddResourceSpawn(ResourceSpawn& spawn, WorldPosition position) {
        spawn.SetLocation(Location(position));
        main_grid[position] = building_id;
    }

    /**
     * Add resource bank to world
     * @param bank resource bank to add
     * @param position where to add bank
     */
    void AddResourceBank(ResourceBank& bank, WorldPosition position) {
        bank.SetLocation(Location(position));
        main_grid[position] = building_id;
    }

    /**
     * Add Town hall to the world
     * @param th town hall
     * @param position where to add spawn
     */
    void AddTownHall(TownHall& th, WorldPosition position) {
        th.SetLocation(Location(position));
        main_grid[position] = building_id;
    }

    void RemoveBuilding(Building& building) {
        if (building.GetLocation().IsPosition()) {
            main_grid[building.GetLocation().AsWorldPosition()] = floor_id; // restore tile
        }
    }
};
}; // namespace cse498
