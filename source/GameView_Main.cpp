/**
 * @file gameview_main.cpp
 * @author Group 17
 *
 * Entry point for the game.
 *
 * This file serves as the example driver demonstrating how the GUI module integrates with the
 * rest of the game system
 *
 * This file initializes and runs the Game class (Game.cpp/Game.hpp), which is the primary file
 * to examine for module integration. Game.cpp contains:
 *      Menu Module Integration (source/Interfaces/GUI/Menu.cpp)
 *      Text Module Integration (source/Interfaces/GUI/Text.cpp)
 *      Image Grid Integration (source/Interfaces/GUI/ImageGrid.cpp)
 *      Image Manager Integration (source/Interfaces/GUI/ImageManager.cpp)
 *      Agent Module Integration via StubAgent (source/core/AgentBase.hpp)
 *      World Module Integration via OverWorld (source/core/OverWorld.hpp)
 *      Full game loop, rendering pipeline, and state management
 *
 * To run:
 *      From root: cmake --build build
 *      After build, run ./source/gameview
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