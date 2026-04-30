/**
 * @file PlayerAgent.cpp
 */

#include "PlayerAgent.hpp"
#include <iostream>
#include "../../core/WorldBase.hpp"
#include "../../core/item/Item.hpp"
#include "../../core/item/ItemWeapon.hpp"

namespace cse498 {

PlayerAgent::PlayerAgent(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {}

bool PlayerAgent::Initialize() {
    mInventory.SetChangeNotifier([this]() { RefreshCombatFromHand(); });
    RefreshCombatFromHand();
    return true;
}

void PlayerAgent::SetStats(const AgentStats& stats) {
    mBaseCombatStats = stats;
    RefreshCombatFromHand();
}

void PlayerAgent::RefreshCombatFromHand() {
    mStats = mBaseCombatStats;
    const Item* hand = mInventory.GetHand();
    if (hand == nullptr || !hand->IsWeapon()) {
        return;
    }
    auto* weapon = dynamic_cast<const ItemWeapon*>(hand);
    if (weapon == nullptr || weapon->IsTool()) {
        return;
    }
    mStats.mAtk = mBaseCombatStats.mAtk + weapon->GetDamage();
    mStats.mRange = weapon->GetRange();
}

size_t PlayerAgent::SelectAction(const WorldGrid& /*grid*/) {
    // This function isn't usable. Don't call it. It needs an override.
    // 1. WorldGrid input is useless since we have that in Entity.
    // 2. This is only designed around this being in the interface implementation, but
    // that creates so many limitations that just annoys me.
    // assert(false); // removed for compatibility with Demos
    return 0;
}

size_t PlayerAgent::SelectPlayerAction(const char input) {
    // After calling this function you need to call DoAction on the result
    // then call SetActionResult for the player.
    // This CANNOT be done here because world is const& and DoAction is not const method.
    // I wish it was... too late now :/

    switch (input) {
        case 'a':
        case 'A':
            return !HasAction("a") ? GetActionID("left") : GetActionID("a");
        case 'w':
        case 'W':
            return !HasAction("w") ? GetActionID("up") : GetActionID("w");
        case 's':
        case 'S':
            return !HasAction("s") ? GetActionID("down") : GetActionID("s");
        case 'd':
        case 'D':
            return !HasAction("d") ? GetActionID("right") : GetActionID("d");
        case 'e':
        case 'E':
            return !HasAction("e") ? GetActionID("interact") : GetActionID("e");
        case 'q':
        case 'Q':
            return GetActionID("q");
        default: {
            const size_t stayAction = GetActionID("stay");
            return stayAction;
        }
    }
}


bool PlayerAgent::SpendGold(std::size_t amount) {
    if (amount > mGold) {
        return false;
    }

    mGold -= amount;
    return true;
}
} // namespace cse498
