/**
 * @file FarmingAgent.cpp
 * @author Group 2
 * @brief Minimal FarmingAgent implementation (trading NPC stub).
 */

#include "FarmingAgent.hpp"

#include <array>
#include <limits>
#include <optional>

#include "../../tools/PathGenerator.hpp"

namespace cse498
{
    bool FarmingAgent::ShouldGoWork() const
    {
        // Farmer only starts going to work if it has a building and enough time has passed
        return mAssignedBuilding != nullptr && mTicksSinceWork >= mWorkInterval;
    }

    bool FarmingAgent::IsAdjacentToWork() const
    {
        if (mAssignedBuilding == nullptr)
        {
            return false;
        }

        const WorldPosition myPosition = GetPosition();
        const WorldPosition buildingPosition = mAssignedBuilding->GetPosition();

        // Work is in range when farmer is standing on a tile next to building
        return myPosition.Up() == buildingPosition || myPosition.Down() == buildingPosition ||
            myPosition.Left() == buildingPosition || myPosition.Right() == buildingPosition;
    }

    bool FarmingAgent::IsAtPosition(const WorldPosition& position) const
    {
        return GetPosition() == position;
    }

    std::optional<WorldPosition> FarmingAgent::FindAdjacentWorkTile(const WorldGrid& grid) const
    {
        if (mAssignedBuilding == nullptr)
        {
            return std::nullopt;
        }

        const WorldPosition buildingPosition = mAssignedBuilding->GetPosition();

        const std::array<WorldPosition, 4> candidates = {
            buildingPosition.Up(),
            buildingPosition.Down(),
            buildingPosition.Left(),
            buildingPosition.Right()
        };

        const WorldPosition myPosition = GetPosition();

        std::optional<WorldPosition> bestTile;
        double bestDistance = std::numeric_limits<double>::infinity();

        for (const WorldPosition& candidate : candidates)
        {
            // only consider walkable tiles
            if (!grid.IsWalkable(candidate))
            {
                continue;
            }

            // Pick closest valid adjacent tile so farmer has a place to path before interaction
            const double distance = PathGenerator::ManhattanDistance(myPosition, candidate);
            if (!bestTile.has_value() || distance < bestDistance)
            {
                bestTile = candidate;
                bestDistance = distance;
            }
        }

        return bestTile;
    }

    std::size_t FarmingAgent::ChooseNextPathAction(const WorldPath& path) const
    {
        // Path needs at least current position and one step
        if (path.Size() < 2)
        {
            return 0;
        }

        const WorldPosition currentPosition = GetPosition();
        const WorldPosition nextPosition = path.At(1);

        // Convert tile in path to movement action
        if (nextPosition == currentPosition.Up()) return GetActionID("up");
        if (nextPosition == currentPosition.Down()) return GetActionID("down");
        if (nextPosition == currentPosition.Left()) return GetActionID("left");
        if (nextPosition == currentPosition.Right()) return GetActionID("right");

        return 0;
    }

    std::size_t FarmingAgent::ChooseStepToward(const WorldGrid& grid, const WorldPosition& target) const
    {
        const PathRequest request(grid);
        const std::optional<WorldPath> path = PathGenerator::FindShortestPath(GetPosition(), target, request);

        // Prefer using actual shortest path so farmer can move around obstacles
        if (path.has_value())
        {
            const std::size_t action = ChooseNextPathAction(*path);
            if (action != 0)
            {
                return action;
            }
        }

        const WorldPosition currentPosition = GetPosition();

        // Fallback to greedy movement if no usable path found
        if (currentPosition.X() < target.X() && grid.IsWalkable(currentPosition.Right()))
        {
            return GetActionID("right");
        }
        if (currentPosition.X() > target.X() && grid.IsWalkable(currentPosition.Left()))
        {
            return GetActionID("left");
        }
        if (currentPosition.Y() < target.Y() && grid.IsWalkable(currentPosition.Down()))
        {
            return GetActionID("down");
        }
        if (currentPosition.Y() > target.Y() && grid.IsWalkable(currentPosition.Up()))
        {
            return GetActionID("up");
        }

        return 0;
    }

    void FarmingAgent::RestockLimitedOffer(const std::string& itemName, int amount)
    {
        if (amount <= 0)
        {
            return;
        }

        TradeOffer* offer = FindOfferMutable(itemName);
        if (offer == nullptr)
        {
            return;
        }

        // Only limited-stock offers get replenished by working
        if (offer->mStockMode == TradeStockMode::Limited)
        {
            offer->mStock += static_cast<std::size_t>(amount);
        }
    }

    void FarmingAgent::OnWorkedAtBuilding()
    {
        int amount = mRestockAmount;

        if (mAssignedBuilding != nullptr)
        {
            // building level can scale how much stock farmer replenishes
            amount += mAssignedBuilding->GetCurrentLevel();
        }

        RestockLimitedOffer(mRestockItemName, amount);

        // Reset work timer and start heading home after work
        mTicksSinceWork = 0;
        mState = FarmerState::ReturningHome;
    }

    std::size_t FarmingAgent::SelectAction(const WorldGrid& grid)
    {
        ++mTicksSinceWork;

        if (mAssignedBuilding == nullptr)
        {
            return 0;
        }

        switch (mState)
        {
        case FarmerState::IdleAtHome:
            {
                // Stay idle until work
                if (!ShouldGoWork())
                {
                    return 0;
                }

                mState = FarmerState::GoingToWork;
                [[fallthrough]];
            }

        case FarmerState::GoingToWork:
            {
                const std::optional<WorldPosition> workTile = FindAdjacentWorkTile(grid);
                if (!workTile.has_value())
                {
                    return 0;
                }

                // Once reached a valid tile next to building start work
                if (IsAtPosition(*workTile) || IsAdjacentToWork())
                {
                    mState = FarmerState::Working;
                    return GetActionID("interact");
                }

                // Otherwise keep moving towards work tile
                return ChooseStepToward(grid, *workTile);
            }

        case FarmerState::Working:
            {
                // Continue returning interact until the world processes it and calls OnWorkedAtBuilding().
                return GetActionID("interact");
            }

        case FarmerState::ReturningHome:
            {
                // Once home, go idle until work
                if (IsAtPosition(mHomePosition))
                {
                    mState = FarmerState::IdleAtHome;
                    return 0;
                }

                return ChooseStepToward(grid, mHomePosition);
            }
        }

        return 0;
    }
}
