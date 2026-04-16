/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Represents the Interactive World module
 * @note Status: PROPOSAL
 **/

#pragma once
#include <array>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include "../../core/WorldBase.hpp"
#include "Building.hpp"
#include "InteractiveWorldInventory.hpp"
#include "NPC.hpp"
#include "ResourceProducer.hpp"

namespace cse498 {
/**
 * World object for the interactive world
 */
class InteractiveWorld : public WorldBase {
protected:
    // World Inventory
    InteractiveWorldInventory m_inventory;

    enum ActionType { REMAIN_STILL = 0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, INTERACT };

    size_t floor_id; ///< Easy access to floor CellType ID.
    size_t wall_id; ///< Easy access to wall CellType ID.

    /// Provide the agent with movement actions.
    void ConfigAgent(AgentBase& agent) override {
        agent.AddAction("up", MOVE_UP);
        agent.AddAction("down", MOVE_DOWN);
        agent.AddAction("left", MOVE_LEFT);
        agent.AddAction("right", MOVE_RIGHT);
        agent.AddAction("interact", INTERACT);
    }

private:
    // NPCs in the scene
    std::vector<std::shared_ptr<NPC>> m_npcs{};
    // ResourceProducers in the scene
    std::vector<std::shared_ptr<ResourceProducer>> m_producers{};
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
        output << m_inventory.GetAmount(ItemType::Wood) << ' ' << ItemTypeToString(ItemType::Wood) << " | "
               << m_inventory.GetAmount(ItemType::Stone) << ' ' << ItemTypeToString(ItemType::Stone) << " | "
               << m_inventory.GetAmount(ItemType::Metal) << ' ' << ItemTypeToString(ItemType::Metal);

        std::cout << output.str() << std::endl;
    }

    template<typename Func>
    void ForEachAdjacentNPC(const std::array<WorldPosition, 4>& neighbors, Func&& func) {
        for (const auto& neighbor: neighbors) {
            for (auto& npc: m_npcs) {
                if (npc->GetPosition() == neighbor) {
                    func(*npc);
                }
            }
        }
    }

    bool IsNPCAt(const WorldPosition& position) const {
        for (const auto& npc: m_npcs) {
            if (npc->GetPosition() == position) {
                return true;
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
     * Get the world inventory object
     * @return reference to world inventory
     */
    InteractiveWorldInventory& GetInventory() { return m_inventory; }
    const InteractiveWorldInventory& GetInventory() const { return m_inventory; }
    /**
     * Have agent perform an action
     * @param agent Agent to perform action on
     * @param action_id type of action to perform
     */
    int DoAction(AgentBase& agent, size_t action_id) override {
        // Determine where the agent is trying to move.
        WorldPosition cur_position = agent.GetLocation().AsWorldPosition();
        const std::array<WorldPosition, 4> neighbors = {cur_position.Up(), cur_position.Down(), cur_position.Left(),
                                                        cur_position.Right()};
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
            case INTERACT: {
                ForEachAdjacentNPC(neighbors, [this](NPC& npc) {
                    const auto upgrade_status = npc.AttemptUpgrade(m_inventory);
                    if (!upgrade_status) {
                        std::cout << "Upgrade failed: "
                                  << Building::UpgradeRejectionTypeToString(upgrade_status.error()) << std::endl;
                        return;
                    }

                    std::cout << "Upgrade succeeded: " << npc.GetUpgradeUI() << std::endl;
                });
                return true;
            }
        }

        // Don't let the agent move off the world or into a non-walkable tile.
        if (!main_grid.IsWalkable(new_position)) {
            return false;
        }

        // Open NPC UI for interface-controlled agents only.
        if (agent.IsInterface()) {
            ForEachAdjacentNPC(neighbors, [](NPC& npc) { npc.Interact(); });
        }

        // Don't walk on NPCs
        if (IsNPCAt(new_position)) {
            return false;
        }

        // Set the agent to its new position.
        agent.SetLocation(new_position);

        return true;
    }

    /**
     * Overlay symbols on screen
     * @return
     */
    std::vector<std::pair<WorldPosition, char>> GetOverlaySymbols() const {
        std::vector<std::pair<WorldPosition, char>> symbols;
        for (const auto& npc: m_npcs) {
            symbols.emplace_back(npc->GetPosition(), npc->GetSymbol());
        }
        return symbols;
    }

    /**
     * Add producer to the world
     * @param producer producer to add
     */
    void AddProducer(std::shared_ptr<ResourceProducer> producer) { m_producers.push_back(producer); }

    /**
     * Add NPC to the world
     * @param npc NPC to add
     */
    void AddNPC(std::shared_ptr<NPC> npc) { m_npcs.push_back(npc); }
};
}; // namespace cse498
