/**
 * @file FarmingAgent.hpp
 * @author Group 2
 * @brief FarmingAgent that periodically goes to work at a building and returns home.
 */
#pragma once

#include <optional>
#include <string>

#include "MerchantAgent.hpp"
#include "../../Worlds/Hub/Building.hpp"
#include "../../tools/WorldPath.hpp"

namespace cse498 {

/** Basic farming NPC agent that can trade and periodically work at an assigned building. */
class FarmingAgent : public MerchantAgent {
public:
    /// States the farmer can be in
    enum class FarmerState { IdleAtHome, GoingToWork, Working, ReturningHome };
private:
    /// Farmer assigned building
    Building* mAssignedBuilding = nullptr;

    /// Where farmer usually is
    WorldPosition mHomePosition{0,0};

    /// Current farmer state
    FarmerState mState = FarmerState::IdleAtHome;

    /// Time since last worked
    int mTicksSinceWork = 0;
    /// How much time should pass between work
    int mWorkInterval = 8;

    /// Amount to restock limited offers
    int mRestockAmount = 2;
    /// Item to restock after work
    std::string mRestockItemName = "wheat";

    /**
     * @return whether the farmer should go to work at assigned building
     */
    [[nodiscard]] bool ShouldGoWork() const;
    /**
     * @return whether assigned building is adjacent to farmer
     */
    [[nodiscard]] bool IsAdjacentToWork() const;

    /**
     * @param position Position to compare against
     * @return whether farmer is exactly at the given position
     */
    [[nodiscard]] bool IsAtPosition(const WorldPosition& position) const;

    /**
     * Finds a walkable tile adjacent to the assigned building.
     *
     * This is used because the farmer should walk next to the building,
     * not onto the building tile itself.
     *
     * @param grid World grid farmer is moving in
     * @return best walkable adjacent tile if one exists
     */
    [[nodiscard]] std::optional<WorldPosition> FindAdjacentWorkTile(const WorldGrid& grid) const;

    /**
     * Converts the next step in a path into a movement action id.
     *
     * @param path Path to follow
     * @return movement action id or 0 if no valid next step
     */
    [[nodiscard]] std::size_t ChooseNextPathAction(const WorldPath& path) const;

    /**
     * Chooses a movement action toward a target position.
     *
     * Uses PathGenerator first, then falls back to simple greedy movement.
     *
     * @param grid World grid farmer is moving in
     * @param target Target position to move toward
     * @return movement action id or 0 if no movement is available
     */
    [[nodiscard]] std::size_t ChooseStepToward(const WorldGrid& grid, const WorldPosition& target) const;

    /**
     * Restocks an offer of limited quantity
     *
     * @param itemName Name of item to be restocked
     * @param amount Amount to restock
     */
    void RestockLimitedOffer(const std::string& itemName, int amount);

public:
    /**
     * Constructs a farming merchant NPC.
     *
     * Shop offers are configured by owning world setup.
     *
     * @param id Unique agent id
     * @param name NPC display name
     * @param world Owning world
     */
    FarmingAgent(std::size_t id, const std::string& name, WorldBase& world) : MerchantAgent(id, name, world) {
        mTradeGreeting = "Fresh crops and seeds today.";
    }

    /**
     * Set an assigned building
     *
     * @param building Building to assign
     */
    void SetAssignedBuilding(Building* building) { mAssignedBuilding = building; }
    /**
     * @return Building agent is assigned to work at
     */
    [[nodiscard]] Building* GetAssignedBuilding() const { return mAssignedBuilding; }

    /**
     * Set where the farmer should return after working.
     *
     * @param position Home position
     */
    void SetHomePosition(const WorldPosition& position) { mHomePosition = position; }

    /**
     * @return Farmer home position
     */
    [[nodiscard]] const WorldPosition& GetHomePosition() const { return mHomePosition; }

    /**
     * Set the number of ticks between work
     *
     * @param ticks Ticks between work
     */
    void SetWorkInterval(int ticks) { mWorkInterval = ticks; }
    /**
     * @return Number of ticks between work
     */
    [[nodiscard]] int GetWorkInterval() const { return mWorkInterval; }

    /**
     * Set the amount to restock offer after farmer works
     *
     * @param amount Amount to restock
     */
    void SetRestockAmount(int amount) { mRestockAmount = amount; }
    /**
     * @return Amount that farmer restocks their limited offer
     */
    [[nodiscard]] int GetRestockAmount() const { return mRestockAmount; }

    /**
     * Set which item gets restocked by string name
     *
     * @param name Name of limited offer item to restock
     */
    void SetRestockItemName(const std::string& name) { mRestockItemName = name; }
    /**
     * @return String name of item to restock
     */
    [[nodiscard]] const std::string& GetRestockItemName() const { return mRestockItemName; }

    /**
     * @return Current farmer state
     */
    [[nodiscard]] FarmerState GetFarmerState() const { return mState; }

    /**
     * Select Farmer's next action
     *
     * @param grid World grid farmer is a part of
     * @return Action id
     */
    [[nodiscard]] std::size_t SelectAction(const WorldGrid& grid) override;

    /**
     * Called by the world when the farmer successfully works at the assigned building.
     */
    void OnWorkedAtBuilding();
};
} // namespace cse498
