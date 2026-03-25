/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A base class interface for all agent types.
 * @note Status: PROPOSAL
 **/

#pragma once

#include <cassert>
#include <memory>
#include <string>
#include <unordered_map>

#include "Entity.hpp"
#include "ItemBase.hpp"
#include "Location.hpp"
#include "WorldGrid.hpp"
#include "WorldPosition.hpp"
#include "../tools/BehaviorTree/BehaviorTree.hpp"

namespace cse498 {

  class AgentBase : public Entity {
  protected:
    /// A map of names to IDs for each available action (ActionMap)
    std::unordered_map<std::string, size_t> mActionMap;

    /// Action results are a 0 for failure); Success is any non-zero value,
    /// which may provide more info about the type of success.
    int mActionResult = 1;

    // TODO: I would guess this will become some struct with a URI reference to image & other metadata
    char mSymbol = '*';

    /// Health and alive state for takeDamage / isAlive / onDeath
    double mHealth = 100.0;
    double mMaxHealth = 100.0;
    bool mAlive = true;

    /// Behavior tree root and blackboard; factory or subclass sets the tree.
    std::unique_ptr<BehaviorTrees::Node> mBehaviorRoot;
    mutable BehaviorTrees::Blackboard mBlackboard;

    /**
     * Called each frame/turn: runs behavior tree tick and component updates.
     * Override in derived classes to add component updates.
     */
    virtual void Tick() {
      if (mBehaviorRoot) {
        BehaviorTrees::ExecutionContext ctx(mBlackboard);
        mBehaviorRoot->Tick(ctx);
      }
    }

  public:
    AgentBase(size_t id, const std::string & name, const WorldBase & world)
      : Entity(id, name, world) { }
    ~AgentBase() override = default;

    // -- Position (thin wrappers over Entity location) --
    [[nodiscard]] WorldPosition GetPosition() const {
      const Location& loc = GetLocation();
      return loc.IsPosition() ? loc.AsWorldPosition() : WorldPosition(0, 0);
    }
    void SetPosition(const WorldPosition& pos) { SetLocation(Location(pos)); }

    // -- Update / lifecycle --
    /// Called by world each frame/turn. Default calls Tick().
    virtual void Update(double /*delta*/) { Tick(); }

    /// Apply damage; at or below zero calls onDeath() and sets alive to false.
    virtual void TakeDamage(double amount) {
      if (!mAlive) return;
      mHealth -= amount;
      if (mHealth <= 0.0) {
        mHealth = 0.0;
        mAlive = false;
        OnDeath();
      }
    }

    [[nodiscard]] bool IsAlive() const { return mAlive; }
    [[nodiscard]] double GetHealth() const { return mHealth; }
    [[nodiscard]] double GetMaxHealth() const { return mMaxHealth; }
    void SetHealth(double h) { mHealth = h; }
    void SetMaxHealth(double h) { mMaxHealth = h; }

    /// Called after agent is added to the world. Override to register or init state.
    virtual void OnSpawn() { }

    /// Called when health reaches zero. Override to spawn loot etc.; world may then destroy agent.
    virtual void OnDeath() { }

    /// Cleanup before removal from world. Override to release resources; world unregisters after.
    virtual void OnDestroy() { }

    /// Access to world (const). Entity stores const WorldBase&.
    [[nodiscard]] const WorldBase& GetWorld() const { return world; }

    /// Behavior tree and blackboard for factory / subclasses
    void SetBehaviorTree(std::unique_ptr<BehaviorTrees::Node> root) { mBehaviorRoot = std::move(root); }
    [[nodiscard]] BehaviorTrees::Blackboard& GetBlackboard() { return mBlackboard; }
    [[nodiscard]] const BehaviorTrees::Blackboard& GetBlackboard() const { return mBlackboard; }

    // Accessors

    [[nodiscard]] char GetSymbol() const { return mSymbol; }
    AgentBase & SetSymbol(char in) { mSymbol = in; return *this; }

    // -- World Interactions --

    /// @brief Run AFTER the world configures the agent, for additional tests or setup.
    /// @return Was the initialization successful?
    virtual bool Initialize() { return true; }

    // -- Entity Overrides --
    bool IsAgent() const override { return true; }

    // -- Action management --

    /// Test if agent already has a specified action.
    [[nodiscard]] bool HasAction(const std::string & action_name) const {
      return mActionMap.count(action_name);
    }

    /// Return an action ID *if* that action exists, otherwise return zero.
    [[nodiscard]] size_t GetActionID(const std::string & action_name) const {
      auto it = mActionMap.find(action_name);
      if (it == mActionMap.end()) return 0;
      return it->second;
    }

    /// Retrieve the result of the most recent action.
    [[nodiscard]] int GetActionResult() const { return mActionResult; }

    //////////////////////////////////////////////////////////////////////////
    //
    //  Agent API -- the member functions below are intended to be called
    //  from a World to:
    //   * provide options of available actions
    //   * request an agent to select their next action
    //   * notify an agent about the result of their latest action
    //   * provide any additional notifications to an agent
    //
    //////////////////////////////////////////////////////////////////////////

    /// Provide info about an action that this agent can take.
    virtual AgentBase & AddAction(const std::string & action_name, size_t action_id) {
      assert(!HasAction(action_name)); // Cannot add existing action name.
      mActionMap[action_name] = action_id;
      return *this;
    }

    /// @brief Decide the next action for this agent to perform. Default: run Tick() then return blackboard "selected_action".
    /// @param grid The current known portions of the WorldGrid
    /// @return ID of the action to perform; (0 is always "no action")
    /// @note Agents can use World API to query for more info (e.g., items, agents, or cell info)
    /// @note Override for custom logic.
    [[nodiscard]] virtual size_t SelectAction(const WorldGrid & grid) {
      (void)grid;
      Tick();
      return mBlackboard.Get<size_t>("selected_action", 0);
    }

    /// Provide the result of this agent's most recent action.
    void SetActionResult(int result) { mActionResult = result; }

    /// @brief Send a notification to this agent
    /// @param message Contents of the notification
    /// @param msg_type Category of message, such as "item_alert", "damage", or "enemy"
    /// @note: For DEVELOPERS - you may want more info provided with notifications.
    virtual void Notify(const std::string & /*message*/,
                        const std::string & /*msg_type*/="none") { }
  };

} // End of namespace cse498
