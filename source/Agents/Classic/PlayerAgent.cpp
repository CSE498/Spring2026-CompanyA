/**
 * @file PlayerAgent.cpp
 */

#include "PlayerAgent.hpp"
#include <iostream>
#include "../../core/WorldBase.hpp"

namespace cse498 {

PlayerAgent::PlayerAgent(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {}

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
            return GetActionID("left");
        case 'w':
        case 'W':
            return GetActionID("up");
        case 's':
        case 'S':
            return GetActionID("down");
        case 'd':
        case 'D':
            return GetActionID("right");
        case 'e':
        case 'E':
            return GetActionID("interact");
        case 'q':
        case 'Q':
            return GetActionID("quit");
        default:
            return GetActionID("stay");
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
