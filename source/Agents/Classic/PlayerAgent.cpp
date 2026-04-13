/**
 * @file PlayerAgent.cpp
 */

#include "PlayerAgent.hpp"
#include "../../core/WorldBase.hpp"
#include <iostream>

namespace cse498 {

PlayerAgent::PlayerAgent(size_t id, const std::string &name, const WorldBase &world)
    : AgentBase(id, name, world) {}

size_t PlayerAgent::SelectAction(const WorldGrid & /*grid*/) {
    // This function isn't usable. Don't call it. It needs an override.
    // 1. WorldGrid input is useless since we have that in Entity.
    // 2. This is only designed around this being in the interface implementation, but
    // that creates so many limitations that just annoys me.
    assert(false);
    return 0;
}

size_t PlayerAgent::SelectPlayerAction(const char input)
{
    // After calling this function you need to call DoAction on the result
    // then call SetActionResult for the player.
    // This CANNOT be done here because world is const& and DoAction is not const method.
    // I wish it was... too late now :/

    switch (input)
    {
        case 'a':
        case 'A':
            return GetActionID("a");
        case 'w':
        case 'W':
            return GetActionID("w");
        case 's':
        case 'S':
            return GetActionID("s");
        case 'd':
        case 'D':
            return GetActionID("d");
        case 'e':
        case 'E':
            return GetActionID("e");
        case 'q':
        case 'Q':
            return GetActionID("q");
        default:
            return GetActionID("stay");
    }

}


bool PlayerAgent::SpendGold(std::size_t amount)
{
    if (amount > mGold)
    {
        return false;
    }

    mGold -= amount;
    return true;
}
} // namespace cse498
