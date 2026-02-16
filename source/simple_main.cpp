/**
 * This file is part of the Fall 2026, CSE 498, section 2, course project.
 * @brief A simplistic main file to demonstrate a system.
 * @note Status: PROPOSAL
 **/

// Include the modules that we will be using.
#include "Agents/PacingAgent.hpp"
#include "Interfaces/TrashInterface.hpp"
#include "Worlds/MazeWorld.hpp"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include "Interfaces/GUI/ImageManager.hpp"

using namespace cse498;

int main()
{
    // Initialize SDL video subsystem
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "SDL_Init Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    // Initialize SDL_image for PNG support
    if ((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != IMG_INIT_PNG) {
        std::cerr << "IMG_Init Error: " << IMG_GetError() << std::endl;
        SDL_Quit();
        return 1;
    }

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow(
        "ImageManager Test Window",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        640, 480,
        SDL_WINDOW_SHOWN
    );
    if (!window) {
        std::cerr << "SDL_CreateWindow Error: " << SDL_GetError() << std::endl;
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create SDL renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        std::cerr << "SDL_CreateRenderer Error: " << SDL_GetError() << std::endl;
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    // Create ImageManager and load image
    ImageManager manager(renderer);
    std::string imagePath = "../tests/Interfaces/GUI/images/ImageManagerTest.png";

    try {
        manager.load_image("testImage", imagePath);
    } catch (const std::runtime_error& e) {
        std::cerr << "Failed to load image: " << e.what() << std::endl;
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
        return 1;
    }

    bool running = true;
    SDL_Event event;
    bool toggleColor = false;
    Uint32 lastSwitch = SDL_GetTicks(); // for background color switch

    while (running) {
        // Event handling
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                running = false;
            }
        }

        // Switch background color every 1 second
        Uint32 now = SDL_GetTicks();
        if (now - lastSwitch >= 1000) {
            toggleColor = !toggleColor;
            lastSwitch = now;
        }

        // Set background color
        if (toggleColor) {
            SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255); // Dark gray
        } else {
            SDL_SetRenderDrawColor(renderer, 200, 200, 200, 255); // Light gray
        }

        SDL_RenderClear(renderer);

        // Draw the image at (100,100)
        if (!manager.draw_image("testImage", 100, 100)) {
            std::cerr << "Failed to draw image!" << std::endl;
        }

        SDL_RenderPresent(renderer);
    }

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}