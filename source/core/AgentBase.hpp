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

#include "../Agents/Classic/AgentStats.hpp"
#include "../tools/ActionLog.hpp"
#include "../tools/BehaviorTree.hpp"
#include "Entity.hpp"
#include "Location.hpp"
#include "WorldGrid.hpp"
#include "WorldPosition.hpp"
#include "../Agents/Classic/PlayerFeatures/Inventory.hpp"

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
    AgentStats mStats;
    bool mAlive = true;

    /// Behavior tree root and blackboard; factory or subclass sets the tree.
    std::unique_ptr<BehaviorTrees::Node> mBehaviorRoot;
    mutable BehaviorTrees::Blackboard mBlackboard;

    // An ActionLog class that tracks the actions made by the agent
    AgentActionLog mActionLog;

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
    AgentBase(size_t id, const std::string& name, const WorldBase& world) : Entity(id, name, world) {}
    ~AgentBase() override = default;

    // -- Position (thin wrappers over Entity location) --
    [[nodiscard]] WorldPosition GetPosition() const {
        const Location& loc = GetLocation();
        return loc.IsPosition() ? loc.AsWorldPosition() : WorldPosition(0, 0);
    }

    // Sets the agent's position and automatically logs a "move" action.
    void SetPosition(const WorldPosition& pos) {
        WorldPosition oldPos = GetPosition();
        SetLocation(Location(pos));
        mActionLog.LogAction(static_cast<int>(GetID()), "move", oldPos, pos);
    }

    // -- Update / lifecycle --
    /// Called by world each frame/turn. Default calls Tick().
    virtual void Update(double /*delta*/) { Tick(); }

    /// Apply damage; at or below zero calls onDeath() and sets alive to false.
    // Applied ActionLog tracking
    virtual void TakeDamage(double amount) {
        if (!mAlive)
            return;
        WorldPosition pos = GetPosition();
        mStats.mHp -= amount;
        if (mStats.mHp <= 0.0) {
            mStats.mHp = 0.0;
            mAlive = false;
            mActionLog.LogAction(static_cast<int>(GetID()), "death", pos, pos);
            OnDeath();
        } else {
            mActionLog.LogAction(static_cast<int>(GetID()), "take_damage", pos, pos);
        }
    }

    // -- ActionLog interface --
    [[nodiscard]] AgentActionLog& GetActionLog() { return mActionLog; }
    [[nodiscard]] const AgentActionLog& GetActionLog() const { return mActionLog; }

    /**
     * @brief Log an action at the agent's current position. Intended for non-movement actions (e.g., attack, interact,
     * wait) where position is the same.
     * @param actionType A string describing the type of action.
     */
    void LogActionNow(const std::string& actionType) {
        WorldPosition pos = GetPosition();
        mActionLog.LogAction(static_cast<int>(GetID()), actionType, pos, pos);
    }

    /**
     * @brief Overload for actions with a distinct destination.
     * @param actionType A string describing the type of action.
     * @param newPos The destination or target position of the action.
     */
    void LogActionNow(const std::string& actionType, const WorldPosition& newPos) {
        mActionLog.LogAction(static_cast<int>(GetID()), actionType, GetPosition(), newPos);
    }

    /**
     * @brief Advance the action log's internal simulation time. Should be called each tick before calling Update()
     * @param time The current simulation time.
     */
    void AdvanceLogTime(double time) { mActionLog.UpdateTime(time); }


    [[nodiscard]] bool IsAlive() const { return mAlive; }
    [[nodiscard]] const AgentStats& GetStats() const { return mStats; }
    virtual void SetStats(const AgentStats& stats) { mStats = stats; }
    [[nodiscard]] double GetCurrentHealth() const { return mStats.mHp; }
    [[nodiscard]] double GetMaxHealth() const { return mStats.mMaxHp; }
    [[nodiscard]] double GetAtk() const { return mStats.mAtk; }
    [[nodiscard]] double GetDef() const { return mStats.mDef; }
    [[nodiscard]] double GetAtkRange() const { return mStats.mRange; }
    [[nodiscard]] size_t GetLevel() const { return mStats.mLevel; }
    /**
     * @brief Cross-team hook used by Group 17 AI agents to identify hostiles without
     *        relying on runtime type info (RTTI) or dynamic_cast chains.
     *
     * @details AI logic such as SmartEnemyAgent::GetTargetPlayerPosition() iterates
     *          @ref WorldBase::GetAgents() and picks a @b non-enemy as its target.
     *          Tagging agents here keeps the AI world-agnostic: any world (OverWorld,
     *          DungeonWorld, AIWorld, etc.) can host a SmartEnemyAgent as long as its
     *          enemies report @c true and its players/NPCs inherit the default @c false.
     *
     * @retval true  This agent should be considered hostile (enemy).
     * @retval false Default; friendly/neutral agent (players, merchants, explorers).
     */
    [[nodiscard]] virtual bool IsEnemy() const { return false; }

        /**
         * sets hp and forces within proper range [0, mMaxHp]
         * @param h - health
         */
        void SetHealth(double h) {
            if (h < 0) h = 0;
            else if (h > mStats.mMaxHp) h = mStats.mMaxHp;
            mStats.mHp = h;
        }

        void SetMaxHealth(double h) { mStats.mMaxHp = h; }

    /// Called after agent is added to the world. Override to register or init state.
    virtual void OnSpawn() {}

    /// Called when health reaches zero. Override to spawn loot etc.; world may then destroy agent.
    virtual void OnDeath() {}

    /// Cleanup before removal from world. Override to release resources; world unregisters after.
    virtual void OnDestroy() {}

    /// Access to world (const). Entity stores const WorldBase&.
    [[nodiscard]] const WorldBase& GetWorld() const { return world; }

    /// Behavior tree and blackboard for factory / subclasses
    void SetBehaviorTree(std::unique_ptr<BehaviorTrees::Node> root) { mBehaviorRoot = std::move(root); }
    [[nodiscard]] BehaviorTrees::Blackboard& GetBlackboard() { return mBlackboard; }
    [[nodiscard]] const BehaviorTrees::Blackboard& GetBlackboard() const { return mBlackboard; }

    // Accessors

    [[nodiscard]] char GetSymbol() const { return mSymbol; }

    AgentBase& SetSymbol(char in) {
        mSymbol = in;
        return *this;
    }

    // -- World Interactions --

    /// @brief Run AFTER the world configures the agent, for additional tests or setup.
    /// @return Was the initialization successful?
    virtual bool Initialize() { return true; }

    // -- Entity Overrides --
    bool IsAgent() const override { return true; }

    // -- Action management --

    /// Test if agent already has a specified action.
    [[nodiscard]] bool HasAction(const std::string& action_name) const { return mActionMap.count(action_name); }

    /// Return an action ID *if* that action exists, otherwise return zero.
    [[nodiscard]] size_t GetActionID(const std::string& action_name) const {
        auto it = mActionMap.find(action_name);
        if (it == mActionMap.end())
            return 0;
        return it->second;
    }

    /// Retrieve the result of the most recent action.
    [[nodiscard]] int GetActionResult() const { return mActionResult; }

    /**
     * Expected overrides if the agent is able to be interacted with
     * This is for PLAYER interaction with the agent. If the player presses E and your agent is nearby
     * what should happen?
     * @return true/false returns
     * ******** IF AN INTERACTION OCCURS YOU MUST RETURN TRUE*********
     */
    virtual bool Interact() { return false; }


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
    virtual AgentBase& AddAction(const std::string& action_name, size_t action_id) {
        assert(!HasAction(action_name)); // Cannot add existing action name.
        mActionMap[action_name] = action_id;
        return *this;
    }

    /// @brief Decide the next action for this agent to perform. Default: run Tick() then return blackboard
    /// "selected_action".
    /// @param grid The current known portions of the WorldGrid
    /// @return ID of the action to perform; (0 is always "no action")
    /// @note Agents can use World API to query for more info (e.g., items, agents, or cell info)
    /// @note Override for custom logic.
    [[nodiscard]] virtual size_t SelectAction([[maybe_unused]] const WorldGrid& grid) {
        Tick();
        return mBlackboard.Get<size_t>("selected_action", 0);
    }

    /// Provide the result of this agent's most recent action.
    void SetActionResult(int result) { mActionResult = result; }

    /// @brief Send a notification to this agent
    /// @param message Contents of the notification
    /// @param msg_type Category of message, such as "item_alert", "damage", or "enemy"
    /// @note: For DEVELOPERS - you may want more info provided with notifications.
    virtual void Notify(const std::string& /*message*/, const std::string& /*msg_type*/ = "none") {}

	virtual bool IsPlayerAgent() const {return false;}

	[[nodiscard]] virtual Inventory& GetInventory() { return sEmptyInventory; }
    [[nodiscard]] virtual const Inventory& GetInventory() const { return sEmptyInventory; }

	private:

	/// Used to enforce polymorphism - all non players share an empty inventory
	/// This helps to avoid the use of dynamic casting within the world classes
	/// NOTE: should check if isPlayerAgent() so that the override getters run instead
	inline static Inventory sEmptyInventory{};
};

} // End of namespace cse498