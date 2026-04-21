/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Agent that collects a resource from a ResourceSpawn and deposits it
 *        at a TownHall. Integrates with the Group 14 interactive-world system.
 */

#pragma once

#include <cstdlib>
#include <queue>
#include <string>
#include <unordered_map>

#include "../../core/AgentBase.hpp"
#include "../../core/WorldGrid.hpp"
#include "../../core/WorldPosition.hpp"
#include "../../Worlds/Hub/ItemType.hpp"
#include "../../Worlds/Hub/ResourceSpawn.hpp"
#include "../../Worlds/Hub/TownHall.hpp"

namespace cse498 {

/**
 * @class FetchAgent
 * @brief Collects a resource from an assigned ResourceSpawn and deposits it at
 *        an assigned TownHall.
 *
 * Per the Group 14 UML, a fetch agent needs:
 *   - the type of resource it gathers (derived from its spawn)
 *   - the current quantity of that resource it is carrying
 *   - the ResourceSpawn it is assigned to
 *   - the TownHall to deposit into
 *
 * Behavior, per SelectAction:
 *   - If carrying 0 and adjacent to the spawn     -> Collect from the spawn.
 *   - If carrying > 0 and adjacent to the townhall -> Deposit at the townhall.
 *   - Otherwise -> BFS one step through walkable tiles toward the current goal.
 *
 * ResourceSpawn and TownHall live on 'B' (building) tiles which are NOT
 * walkable. The FetchAgent therefore targets a tile adjacent to the goal
 * rather than the goal tile itself.
 *
 * Header-only to match the style of the Hub classes it integrates with
 * (TownHall, ResourceSpawn, Building).
 */
class FetchAgent : public AgentBase {
public:
    FetchAgent(size_t id, const std::string& name, const WorldBase& world) :
        AgentBase(id, name, world) {}

    /// @brief Assign the ResourceSpawn this agent harvests from. Also sets the
    ///        agent's resource type to match the spawn.
    FetchAgent& SetSpawn(ResourceSpawn& spawn) {
        m_spawn = &spawn;
        m_itemType = spawn.GetItemType();
        return *this;
    }

    /// @brief Assign the TownHall this agent deposits into.
    FetchAgent& SetTownHall(TownHall& townHall) {
        m_townHall = &townHall;
        return *this;
    }

    [[nodiscard]] const ResourceSpawn* GetSpawn() const { return m_spawn; }
    [[nodiscard]] const TownHall* GetTownHall() const { return m_townHall; }
    [[nodiscard]] ItemType GetItemType() const { return m_itemType; }
    [[nodiscard]] int GetCarryQuantity() const { return m_carryQuantity; }
    [[nodiscard]] int GetTotalDelivered() const { return m_totalDelivered; }

    [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override {
        if (m_spawn == nullptr || m_townHall == nullptr) {
            return 0;
        }
        if (!GetLocation().IsPosition()) {
            return 0;
        }

        const WorldPosition myPos = GetLocation().AsWorldPosition();

        const AgentBase* goalAgent = (m_carryQuantity > 0)
                                         ? static_cast<const AgentBase*>(m_townHall)
                                         : static_cast<const AgentBase*>(m_spawn);
        if (!goalAgent->GetLocation().IsPosition()) {
            return 0;
        }
        const WorldPosition goalPos = goalAgent->GetLocation().AsWorldPosition();

        if (IsAdjacent(myPos, goalPos)) {
            if (m_carryQuantity > 0) {
                m_townHall->DepositResource(m_itemType, m_carryQuantity);
                m_totalDelivered += m_carryQuantity;
                m_carryQuantity = 0;
                LogActionNow("deposit");
            } else {
                m_carryQuantity = m_spawn->Collect();
                LogActionNow("collect");
            }
            return 0; // stay still; the turn was spent on the collect/deposit
        }

        return NextStepToward(grid, myPos, goalPos);
    }

private:
    ResourceSpawn* m_spawn = nullptr;
    TownHall* m_townHall = nullptr;
    ItemType m_itemType = ItemType::Wood;
    int m_carryQuantity = 0;
    int m_totalDelivered = 0;

    [[nodiscard]] static bool IsAdjacent(const WorldPosition& a, const WorldPosition& b) {
        const long long dx =
            static_cast<long long>(a.CellX()) - static_cast<long long>(b.CellX());
        const long long dy =
            static_cast<long long>(a.CellY()) - static_cast<long long>(b.CellY());
        return std::llabs(dx) + std::llabs(dy) == 1;
    }

    /// @brief BFS through walkable tiles to find the first step that moves
    ///        `from` closer to a tile adjacent to `goal`. The goal tile itself
    ///        is treated as reachable even though it is non-walkable, so the
    ///        expansion terminates on the tile adjacent to the building.
    [[nodiscard]] size_t NextStepToward(const WorldGrid& grid, WorldPosition from,
                                        WorldPosition goal) const {
        if (from == goal) {
            return 0;
        }

        std::unordered_map<WorldPosition, WorldPosition> parent;
        std::queue<WorldPosition> frontier;
        parent.emplace(from, from);
        frontier.push(from);

        WorldPosition reached = from;
        bool found = false;

        while (!frontier.empty() && !found) {
            const WorldPosition cur = frontier.front();
            frontier.pop();

            const WorldPosition neighbors[4] = {cur.Up(), cur.Down(), cur.Left(), cur.Right()};
            for (const WorldPosition& n : neighbors) {
                if (parent.find(n) != parent.end()) {
                    continue;
                }
                if (n == goal) {
                    parent.emplace(n, cur);
                    reached = n;
                    found = true;
                    break;
                }
                if (!grid.IsWalkable(n)) {
                    continue;
                }
                parent.emplace(n, cur);
                frontier.push(n);
            }
        }

        if (!found) {
            return 0;
        }

        WorldPosition step = reached;
        while (true) {
            auto it = parent.find(step);
            if (it == parent.end()) {
                return 0;
            }
            if (it->second == from) {
                break;
            }
            step = it->second;
        }

        if (step == from.Up()) {
            return GetActionID("up");
        }
        if (step == from.Down()) {
            return GetActionID("down");
        }
        if (step == from.Left()) {
            return GetActionID("left");
        }
        if (step == from.Right()) {
            return GetActionID("right");
        }
        return 0;
    }
};

} // namespace cse498
