#pragma once
#include "../core/WorldBase.hpp"
#include "../core/InteractiveWorld/Building.hpp"
#include "../core/InteractiveWorld/InteractiveWorldInventory.hpp"
#include "../core/InteractiveWorld/NPC.hpp"
#include "../core/InteractiveWorld/ResourceProducer.hpp"
#include <memory>
#include <vector>
#include <iostream>


namespace cse498
{
	class InteractiveWorld : public WorldBase
	{
	protected:

		InteractiveWorldInventory m_inventory;

	    enum ActionType { REMAIN_STILL=0, MOVE_UP, MOVE_DOWN, MOVE_LEFT, MOVE_RIGHT, INTERACT};

	    size_t floor_id; ///< Easy access to floor CellType ID.
	    size_t wall_id;  ///< Easy access to wall CellType ID.

	    /// Provide the agent with movement actions.
	    void ConfigAgent(AgentBase & agent) override {
		    agent.AddAction("up", MOVE_UP);
		    agent.AddAction("down", MOVE_DOWN);
		    agent.AddAction("left", MOVE_LEFT);
		    agent.AddAction("right", MOVE_RIGHT);
		    agent.AddAction("interact", INTERACT);
	    }

	private:
	    std::vector<std::shared_ptr<NPC>> m_npcs{};
	    std::vector<std::shared_ptr<ResourceProducer>> m_producers{};
	    /**
	     * Update world logic
	     */
	    void UpdateWorld() override
	    {
		for (auto producer : m_producers)
		{
			producer->Update();
		}

		PrintInventory();
	    }

	    // Temporary Debug
	    std::string ItemTypeToString(const ItemType& itemType)
	    {
		    if (itemType == ItemType::Wood)
			    return "Wood";
		    else if (itemType == ItemType::Stone)
			    return "Stone";
		    else if (itemType == ItemType::Metal)
			    return "Metal";
		    return "";
	    }

	    // Temporary Debug
	    void PrintInventory()
	    {
		    std::string inv{};

		    inv += std::to_string(m_inventory.GetAmount(ItemType::Wood)) + " " + ItemTypeToString(ItemType::Wood) + " | ";
		    inv += std::to_string(m_inventory.GetAmount(ItemType::Stone)) + " " + ItemTypeToString(ItemType::Stone) + " | ";
		    inv += std::to_string(m_inventory.GetAmount(ItemType::Metal)) + " " + ItemTypeToString(ItemType::Metal);

		    std::cout << inv << std::endl;
	    }

	public:
	    InteractiveWorld()
	    {
		    floor_id = main_grid.AddCellType("floor", "Floor that agents can walk on.", ' ');
		    wall_id  = main_grid.AddCellType("wall",  "Impenetrable wall.",             '#');

		    main_grid.Load(std::vector<std::string>{"#######################",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#                     #",
						            "#######################"} );
	    }

	    ~InteractiveWorld() = default;

		InteractiveWorldInventory& GetInventory() {
			return m_inventory;
		}

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
			    case INTERACT:
			    {
				    std::array<WorldPosition, 4> neighbors = {
				        cur_position.Up(),
				        cur_position.Down(),
				        cur_position.Left(),
				        cur_position.Right()
				    };

				    for (const auto & neighbor : neighbors) {
					    for (auto & npc : m_npcs) {
						    if (npc->GetPosition() == neighbor) {
							    npc->Interact();
						    }
					    }
				    }
				    return true;
			    };
		    }

		    // Don't let the agent move off the world or into a wall.
		    if (!main_grid.IsValid(new_position)) { return false; }
		    if (main_grid[new_position] == wall_id) { return false; }
		    // Don't walk on NPCs
		    for (const auto & npc : m_npcs) {
			    if (npc->GetPosition() == new_position) { return false; }
		    }

		    // Set the agent to its new position.
		    agent.SetLocation(new_position);

		    return true;
	    }

	    /**
	     * Overlay symbols on screen
	     * @return
	     */
	    std::vector<std::pair<WorldPosition, char>> GetOverlaySymbols() const override {
		    std::vector<std::pair<WorldPosition, char>> symbols;
		    for (const auto & npc : m_npcs) {
			    symbols.emplace_back(npc->GetPosition(), npc->GetSymbol());
		    }
		    return symbols;
	    }

	    /**
	     * Add producer to the world
	     * @param producer producer to add
	     */
	    void AddProducer(std::shared_ptr<ResourceProducer> producer)
	    {
		    m_producers.push_back(producer);
	    }

	    /**
	     * Add NPC to the world
	     * @param npc NPC to add
	     */
	    void AddNPC(std::shared_ptr<NPC> npc)
	    {
		    m_npcs.push_back(npc);
	    }
	};
};
