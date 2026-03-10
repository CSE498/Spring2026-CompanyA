/**
* @file gameview_main.cpp
 * @author Group 17
 *
 * Temporary entry point for testing GameView window.
 */

#include "Interfaces/GUI/GameView.hpp"
#include <SDL2/SDL.h>

int main()
{
    cse498::GameView view("Slay The Dungeon", 800, 600);

    if (!view.Initialize()) {
        return 1;
    }

    bool running = true;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = false;
        }
        view.Clear();
        view.Present();
    }

    return 0;
}