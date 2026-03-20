/**
* @file gameview_main.cpp
 * @author Group 17
 *
 * Entry point for the game.
 */

#include "core/Game.hpp"

int main()
{
    cse498::Game game("Slay The Dungeon", 800, 600);
    if (!game.Initialize()) return 1;
    game.Run();
    return 0;
}