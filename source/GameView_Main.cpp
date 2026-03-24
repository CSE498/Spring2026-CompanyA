/**
 * @file gameview_main.cpp
 * @author Group 17
 *
 * Entry point for the game.
 */

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include "core/Game.hpp"

int main()
{
    cse498::Game game("Slay The Dungeon", 800, 600);
    if (!game.Initialize()) return 1;
    game.Run();
    return 0;
}