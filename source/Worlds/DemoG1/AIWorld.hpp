/**
 * @file AIWorld.hpp
 * @brief MazeWorld extension with agent HP/attack/heal state, floor loot, and
 *        enemies; extra actions for pickup, heal, and directional attacks.
 *
 * @details Turn loop: agents act, then enemy AI runs. End conditions: all
 *          enemies dead, all agents dead, or turn limit. See docs/Group1.md.
 */
#pragma once

#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../MazeWorld.hpp"

namespace cse498 {

/// Pickup item on the floor: weapon (boosts attack) or heal (grants heal
/// charges).
class LootItem : public ItemBase {
public:
    enum class ItemKind { Weapon, Heal };

    LootItem(size_t id, const std::string& name, const WorldBase& world, ItemKind kind, int power);

    [[nodiscard]] ItemKind GetKind() const;
    [[nodiscard]] int GetPower() const;
    [[nodiscard]] bool IsConsumed() const;
    void Consume();

private:
    ItemKind mKind;
    int mPower;
    bool mConsumed = false;
};

/// Combat/loot maze demo world; pairs with TrailblazerAgent or similar.
class AIWorld : public MazeWorld {
protected:
    enum AIActionType { PICKUP_ITEM = 5, USE_HEAL, ATTACK_UP, ATTACK_DOWN, ATTACK_LEFT, ATTACK_RIGHT };

    struct EnemyState {
        size_t mID = 0;
        WorldPosition mPosition{};
        int mHP = 0;
        int mAttack = 1;
        bool mAlive = true;
    };

public:
    struct AgentState {
        int mHP = 12;
        int mMaxHP = 12;
        int mAttack = 2;
        int mHealCharges = 0;
        int mScore = 0;
    };

    struct VisibleItem {
        size_t mItemID = 0;
        WorldPosition mPosition{};
        LootItem::ItemKind mKind = LootItem::ItemKind::Weapon;
        int mPower = 0;
    };

protected:
    std::vector<EnemyState> mEnemies;
    std::unordered_map<size_t, AgentState> mAgentState;
    size_t mMaxTurns = 500;
    size_t mCurrentTurn = 0;
    bool mVerboseMode = true;
    bool mStepMode = true;

    /// @return false if the user requested quit (e.g. entered `q`), true to
    /// continue.
    bool WaitForStep();
    [[nodiscard]] std::string ActionName(size_t action_id) const;
    void PrintWorldState(const std::string& header) const;
    [[nodiscard]] bool IsWalkable(WorldPosition pos) const;
    [[nodiscard]] bool IsEnemyAt(WorldPosition pos, size_t* enemy_index = nullptr) const;
    [[nodiscard]] bool IsAgentAt(WorldPosition pos, size_t skip_agent_id = static_cast<size_t>(-1)) const;
    [[nodiscard]] std::optional<size_t> ItemOnFloor(WorldPosition pos) const;
    void HandleEnemyTurn(EnemyState& enemy);
    void ConfigAgent(AgentBase& agent) override;

public:
    AIWorld();

    void SetVerboseMode(bool in) { mVerboseMode = in; }
    void SetStepMode(bool in) { mStepMode = in; }
    [[nodiscard]] const std::vector<EnemyState>& GetEnemies() const { return mEnemies; }
    [[nodiscard]] const AgentState& GetAgentState(size_t id) const { return mAgentState.at(id); }

    [[nodiscard]] std::vector<VisibleItem> GetFloorItems() const;
    [[nodiscard]] bool IsEnemyAtPosition(WorldPosition pos) const { return IsEnemyAt(pos); }
    [[nodiscard]] bool IsAgentAtPosition(WorldPosition pos, size_t skip_agent_id = static_cast<size_t>(-1)) const {
        return IsAgentAt(pos, skip_agent_id);
    }

    int DoAction(AgentBase& agent, size_t action_id) override;
    void RunAgents() override;
    void UpdateWorld() override;
    void Run() override;
};

} // namespace cse498
