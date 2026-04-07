/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class for all World modules.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <type_traits>

#include "AgentBase.hpp"
#include "ItemBase.hpp"
#include "WorldGrid.hpp"
#include "../Agents/PlayerAgent.hpp"

namespace cse498 {

  using item_ptr_t = std::unique_ptr<ItemBase>;
  using item_set_t = std::vector<item_ptr_t>;
  using agent_ptr_t = std::unique_ptr<AgentBase>;
  using agent_set_t = std::vector<agent_ptr_t>;

  class WorldBase {
    private:
      /// current agent ID for this world. (reserve 0? could be nice..? Player?)
      /// Note this also fixed a potential bug -- It used to be defined by size of agent_size but that changes
      /// up and down which gives multiple duplicate ids to multiple agents..
      size_t mAgentIdIndex = 0;

    protected:
    /// NOTE: derived worlds may choose to have more than one grid.
    WorldGrid main_grid;                 ///< Main grid for this world

    item_set_t item_set;    ///< Vector of pointers to non-agent entities (ItemBase)
    agent_set_t agent_set;  ///< Vector of pointers to agent entities (AgentBase)
    /// The main player stored separately from the agents and has id = 0
    PlayerAgent* mPlayer;

    bool mRunOver = false;  ///< Are we finished executing and now shutting down?



    /// Helper function that is run whenever a new agent is created.
    /// @note Override this function to provide agents with actions or other setup.
    virtual void ConfigAgent(AgentBase & /* agent */) { }


  public:
    WorldBase()
    {
      auto player = std::make_unique<PlayerAgent>(GetNextAgentId(), "Player", *this);
      AddAgent(std::move(player));
      mPlayer = dynamic_cast<PlayerAgent*>(agent_set[0].get());
      assert(mPlayer);
    }
    virtual ~WorldBase() = default;

    // -- Accessors --

    /// Get the total number of NON-agent entities
    [[nodiscard]] size_t GetNumItems() const { return item_set.size(); }

    /// Get the total number of AGENT entities
    [[nodiscard]] size_t GetNumAgents() const { return agent_set.size(); }

    /// Return a reference to an Item with a given ID.
    [[nodiscard]] ItemBase & GetItem(size_t id) {
      assert(id < item_set.size());
      return *item_set[id];
    }

    /// Return a CONST reference to an Item with a given ID.
    [[nodiscard]] const ItemBase & GetItem(size_t id) const {
      assert(id < item_set.size());
      return *item_set[id];
    }

    /// Return a reference to an Agent with a given ID.
    [[nodiscard]] AgentBase & GetAgent(size_t id) {
      AgentBase* agent = TryGetAgent(id);
      assert(agent != nullptr);
      return *agent;
    }

    /// Return a CONST reference to an Agent with a given ID.
    [[nodiscard]] const AgentBase & GetAgent(size_t id) const {
      const AgentBase* agent = TryGetAgent(id);
      assert(agent != nullptr);
      return *agent;
    }

      /**
       *Finds the current vector index for an agent with the given stable ID.
       *
       * Agent IDs do not change when dead agents are erased from agent_set,
       * so ID lookup must not assume ID == vector index.
       */
      [[nodiscard]] size_t FindAgentIndexById(size_t id) const {
      for (size_t i = 0; i < agent_set.size(); ++i) {
        if (agent_set[i] && agent_set[i]->GetID() == id) {
          return i;
        }
      }
      return agent_set.size();
    }

    [[nodiscard]] bool HasAgent(size_t id) const {
      return FindAgentIndexById(id) != agent_set.size();
    }

    [[nodiscard]] AgentBase* TryGetAgent(size_t id) {
      const size_t index = FindAgentIndexById(id);
      if (index == agent_set.size()) {
        return nullptr;
      }
      return agent_set[index].get();
    }

    [[nodiscard]] const AgentBase* TryGetAgent(size_t id) const {
      const size_t index = FindAgentIndexById(id);
      if (index == agent_set.size()) {
        return nullptr;
      }
      return agent_set[index].get();
    }

      /**
       * Accesses an agent by its current storage position in agent_set.
       *
       * This is intended for safe iteration over the internal agent container.
       * It should not be used when stable ID lookup is required.
       */
      [[nodiscard]] AgentBase & GetAgentByIndex(size_t index) {
      assert(index < agent_set.size());
      return *agent_set[index];
    }

    [[nodiscard]] const AgentBase & GetAgentByIndex(size_t index) const {
      assert(index < agent_set.size());
      return *agent_set[index];
    }

      /**
       * Gets the player. Player shouldn't be nullptr (it is asserted)
       * @return actual player pointer
       */
      [[nodiscard]] PlayerAgent* GetPlayer() const
    {
      /*
       * It shouldn't be possible for player to ever be null. Dead ==> IsAlive is false
       * Player object shouldn't need to be recreated, if so then design a function for that
       * or check whole project for UB
       */
      assert(mPlayer != nullptr && "Player got set to nullptr somehow?");
      return mPlayer;
    }

      /**
       * Gets player position -- Will return even if player is dead (like minecraft)
       * @return Player position
       */
      [[nodiscard]] WorldPosition GetPlayerPosition() const
    {
        return mPlayer->GetLocation().AsWorldPosition();
    }

    /// Return an editable version of the current grid for this world (main_grid by default) 
    virtual WorldGrid & GetGrid() { return main_grid; }

    /// Return the current grid for this world (main_grid by default) 
    [[nodiscard]] virtual const WorldGrid & GetGrid() const { return main_grid; }

    /// Determine if the run has ended.
    [[nodiscard]] virtual bool IsRunOver() const { return mRunOver; }

    size_t GetNextAgentId() { return mAgentIdIndex++; }

    // -- Agent Management --

    /// @brief Build a new agent of the specified type
    /// @tparam AGENT_T The type of agent to build
    /// @param agent_name The name of this agent
    /// @return A reference to the newly created agent
    template <typename AGENT_T>
    AGENT_T & AddAgent(std::string agent_name="None") {
      auto agent_ptr = std::make_unique<AGENT_T>(GetNextAgentId(), agent_name, *this);
      return AddAgent(std::move(agent_ptr));
    }

    /**
     * Other build system for agents. Agents that are not defined as classes, create the agent from the factory
     * and pass it into here to add the agent.
     * @param agent
     * @return
     */
    template<typename AGENT_T>
    AGENT_T & AddAgent(std::unique_ptr<AGENT_T> agent)
    {
      static_assert(std::is_base_of_v<AgentBase, AGENT_T> == true);
      if (!agent)
        agent = std::make_unique<AGENT_T>(GetNextAgentId(), "None", *this);
      AGENT_T & agent_ref = *agent;
      ConfigAgent(*agent);
      if (agent->Initialize() == false) {
        std::cerr << "Failed to initialize agent '" << agent->GetName() << "'." << std::endl;
      }
      agent_set.emplace_back(std::move(agent));
      return agent_ref;
    }

    // -- Action Management --

    /// @brief Central function for an agent to take any action
    /// @param agent The specific agent taking the action
    /// @param action The id of the action to take
    /// @return The result of this action (usually 0/1 to indicate success)
    /// @note Thus function must be overridden in any derived world.
    virtual int DoAction(AgentBase & agent, size_t action_id) = 0;

    /// @brief Step through each agent giving them a chance to take an action.
    /// @note Override function to control execution order of agents.
    /// @note Override function to control which grid each agent receives.
    virtual void RunAgents() {
      for (const auto & agent_ptr : agent_set) {
        size_t action_id = agent_ptr->SelectAction(main_grid);
        int result = DoAction(*agent_ptr, action_id);
        agent_ptr->SetActionResult(result);
      }
    }

    /// Remove agents that are dead: call OnDestroy() then erase from agent_set.
    virtual void RemoveDeadAgents() {
      auto it = agent_set.begin();
      while (it != agent_set.end()) {
        if (!(*it)->IsAlive()) {
          if (it->get() == mPlayer) {
            it++; // we can declare the player dead but not null.
            continue;
          }
          (*it)->OnDestroy();
          it = agent_set.erase(it);
        } else {
          ++it;
        }
      }
    }

    /// @brief UpdateWorld() is run after every agent has a turn.
    /// Override this function to manage background events for a world.
    /// (E.g., weather, growth, regular physics, etc.)
    virtual void UpdateWorld() { }

    /// @brief Run all agents repeatedly until an end condition is met.
    virtual void Run() {
      mRunOver = false;
      while (!mRunOver) {
        RunAgents();
        RemoveDeadAgents();
        UpdateWorld();
      }
    }


    //////////////////////////////////////////////////////////////////////////
    //
    //  World API -- the member functions below are intended to be called
    //  from Agents to get more information about their senses and options.
    //
    //////////////////////////////////////////////////////////////////////////
    
    // Provide a vector of IDs for other agents that the input agent is aware of.
    // (If not overridden, return ALL agents.)
    virtual std::vector<size_t> GetKnownAgents([[maybe_unused]] const AgentBase & agent) const {
      std::vector<size_t> out_ids;
      for (const agent_ptr_t & ptr : agent_set) out_ids.push_back(ptr->GetID());
      return out_ids;
    }

    // Provide a vector of IDs for items that the input agent is aware of.
    // (If not overridden, return ALL items.)
    std::vector<size_t> GetKnownItems([[maybe_unused]] const AgentBase & agent) const {
      std::vector<size_t> out_ids;
      for (const item_ptr_t & ptr : item_set) out_ids.push_back(ptr->GetID());
      return out_ids;
    }
  };

} // End of namespace cse498
