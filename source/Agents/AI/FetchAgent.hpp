/**
 * This file is for the Fall 2026 CSE 498 section 2 Capstone project.
 * @brief Agent that shuttles between an origin point and a deposit point.
 */

#pragma once

#include <cstdlib>
#include <functional>
#include <queue>
#include <string>
#include <unordered_map>
#include <utility>

#include "../../core/AgentBase.hpp"
#include "../../core/WorldGrid.hpp"
#include "../../core/WorldPosition.hpp"
#include "../../Worlds/Hub/ItemType.hpp"
#include "../../Worlds/Hub/ResourceBank.hpp"
#include "../../Worlds/Hub/ResourceSpawn.hpp"
#include "../../Worlds/Hub/TownHall.hpp"

namespace cse498 {

/**
 * @class FetchAgent
 * @brief Minimal hauling agent that travels between two endpoints.
 *
 * The agent is intentionally generic:
 *   - `origin` is where it travels when empty
 *   - `deposit point` is where it travels when carrying cargo
 *   - optional callbacks define what happens at each endpoint
 *
 * By default, reaching the origin will:
 *   - collect from a ResourceSpawn
 *   - withdraw from a ResourceBank
 *   - otherwise fall back to a synthetic carry quantity of 1
 *
 * If a resource source is empty, the agent waits adjacent to it and retries on
 * later turns. Reaching the deposit point unloads everything. Convenience
 * wrappers keep the original ResourceSpawn -> TownHall flow available for the
 * Group 14 demo.
 *
 * ResourceSpawn and building-like endpoints typically live on non-walkable
 * tiles, so the agent paths toward a tile adjacent to its current goal.
 */
class FetchAgent : public AgentBase {
public:
    using EndpointAction = std::function<void(FetchAgent&)>;

    FetchAgent(size_t id, const std::string& name, const WorldBase& world) :
        AgentBase(id, name, world) {}

    /// @brief Assign the endpoint this agent targets while empty.
    FetchAgent& SetOrigin(AgentBase& origin) {
        m_origin = &origin;
        return *this;
    }

    /// @brief Assign the endpoint this agent targets while carrying cargo.
    FetchAgent& SetDepositPoint(AgentBase& depositPoint) {
        m_depositPoint = &depositPoint;
        return *this;
    }

    /// @brief Set logic to run when the agent reaches the origin.
    FetchAgent& SetOnOriginReached(EndpointAction action) {
        m_onOriginReached = std::move(action);
        return *this;
    }

    /// @brief Set logic to run when the agent reaches the deposit point.
    FetchAgent& SetOnDepositReached(EndpointAction action) {
        m_onDepositReached = std::move(action);
        return *this;
    }

    /// @brief Convenience wrapper for the original spawn-based setup.
    FetchAgent& SetSpawn(ResourceSpawn& spawn) {
        SetOrigin(spawn);
        SetItemType(spawn.GetItemType());
        return SetOnOriginReached([&spawn](FetchAgent& agent) {
            agent.SetItemType(spawn.GetItemType());
            agent.SetCarryQuantity(spawn.Collect());
        });
    }

    /// @brief Convenience wrapper for the original TownHall-based setup.
    FetchAgent& SetTownHall(TownHall& townHall) {
        SetDepositPoint(townHall);
        return SetOnDepositReached([&townHall](FetchAgent& agent) {
            const int quantity = agent.GetCarryQuantity();
            if (quantity <= 0) {
                return;
            }

            townHall.DepositResource(agent.GetItemType(), quantity);
            agent.AddDelivered(quantity);
            agent.SetCarryQuantity(0);
        });
    }

    [[nodiscard]] const AgentBase* GetOrigin() const { return m_origin; }
    [[nodiscard]] const AgentBase* GetDepositPoint() const { return m_depositPoint; }
    [[nodiscard]] const ResourceSpawn* GetSpawn() const { return dynamic_cast<const ResourceSpawn*>(m_origin); }
    [[nodiscard]] const TownHall* GetTownHall() const { return dynamic_cast<const TownHall*>(m_depositPoint); }
    [[nodiscard]] ItemType GetItemType() const { return m_itemType; }
    [[nodiscard]] int GetCarryQuantity() const { return m_carryQuantity; }
    [[nodiscard]] int GetTotalDelivered() const { return m_totalDelivered; }

    FetchAgent& SetItemType(ItemType itemType) {
        m_itemType = itemType;
        return *this;
    }

    FetchAgent& SetCarryQuantity(int quantity) {
        m_carryQuantity = (quantity < 0) ? 0 : quantity;
        return *this;
    }

    FetchAgent& AddDelivered(int amount) {
        if (amount > 0) {
            m_totalDelivered += amount;
        }
        return *this;
    }

    [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override {
        if (m_origin == nullptr || m_depositPoint == nullptr) {
            return 0;
        }
        if (!GetLocation().IsPosition()) {
            return 0;
        }

        const WorldPosition myPos = GetLocation().AsWorldPosition();
        const AgentBase* goalAgent =
            (m_carryQuantity > 0) ? static_cast<const AgentBase*>(m_depositPoint)
                                  : static_cast<const AgentBase*>(m_origin);

        if (!goalAgent->GetLocation().IsPosition()) {
            return 0;
        }

        const WorldPosition goalPos = goalAgent->GetLocation().AsWorldPosition();

        if (IsAdjacent(myPos, goalPos)) {
            if (m_carryQuantity > 0) {
                if (m_onDepositReached) {
                    m_onDepositReached(*this);
                } else {
                    PerformDefaultDeposit();
                }
                LogActionNow("deposit");
            } else {
                if (m_onOriginReached) {
                    m_onOriginReached(*this);
                } else {
                    PerformDefaultPickup();
                }
                LogActionNow("pickup");
            }
            return 0;
        }

        return NextStepToward(grid, myPos, goalPos);
    }

private:
    AgentBase* m_origin = nullptr;
    AgentBase* m_depositPoint = nullptr;
    EndpointAction m_onOriginReached;
    EndpointAction m_onDepositReached;
    ItemType m_itemType = ItemType::Wood;
    int m_carryQuantity = 0;
    int m_totalDelivered = 0;

    void PerformDefaultPickup() {
        if (auto* spawn = dynamic_cast<ResourceSpawn*>(m_origin); spawn != nullptr) {
            SetItemType(spawn->GetItemType());
            SetCarryQuantity(spawn->Collect());
            return;
        }

        if (auto* bank = dynamic_cast<ResourceBank*>(m_origin); bank != nullptr) {
            const int available = static_cast<int>(bank->GetStoredAmount(m_itemType));
            if (available <= 0) {
                SetCarryQuantity(0);
                return;
            }

            if (bank->WithdrawResource(m_itemType, available)) {
                SetCarryQuantity(available);
            }
            return;
        }

        SetCarryQuantity(1);
    }

    void PerformDefaultDeposit() {
        if (m_carryQuantity <= 0) {
            return;
        }

        if (auto* townHall = dynamic_cast<TownHall*>(m_depositPoint); townHall != nullptr) {
            townHall->DepositResource(m_itemType, m_carryQuantity);
            AddDelivered(m_carryQuantity);
            SetCarryQuantity(0);
            return;
        }

        if (auto* bank = dynamic_cast<ResourceBank*>(m_depositPoint); bank != nullptr) {
            if (bank->DepositResource(m_itemType, m_carryQuantity)) {
                AddDelivered(m_carryQuantity);
                SetCarryQuantity(0);
            }
            return;
        }

        AddDelivered(m_carryQuantity);
        SetCarryQuantity(0);
    }

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
    ///        expansion terminates on the tile adjacent to the endpoint.
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
