/**
 * @file PlayerAgent.cpp
 */

#include "PlayerAgent.hpp"

#include <iostream>

namespace cse498 {

PlayerAgent::PlayerAgent(size_t id, const std::string &name, const WorldBase &world)
    : InterfaceBase(id, name, world) {}

size_t PlayerAgent::SelectAction(const WorldGrid & /*grid*/) {
    char input = '\0';
    std::cout << "WASD move, E interact with NPC/enemy, X quit.\n> ";
    std::cin >> input;
    switch (input) {
    case 'w':
    case 'W':
        return GetActionID("up");
    case 's':
    case 'S':
        return GetActionID("down");
    case 'a':
    case 'A':
        return GetActionID("left");
    case 'd':
    case 'D':
        return GetActionID("right");
    case 'e':
    case 'E':
        return GetActionID("interact");
    case 'x':
    case 'X':
        return GetActionID("quit");
    default:
        return 0;
    }
}

} // namespace cse498
