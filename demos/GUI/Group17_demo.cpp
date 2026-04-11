/**
 * @file Group17_demo.cpp
 * @author Group 17
 * Demo for Group 17's Game and GameView classes.
 * Demonstrates instantiation, initialization, state management,
 * and the render pipeline for the Slay The Dungeon game.
 * Made via reference to Claude Sonnet 4.6
 */

#define SDL_MAIN_HANDLED
#include "../source/core/Game.hpp"
#include "../source/Interfaces/GUI/GameView.hpp"
#include "../source/Interfaces/GUI/ErrorManager.hpp"
#include <iostream>
#include <SDL2/SDL.h>

// -----------------------------------------------------------------------
//  Helper: pause and print a step header so the demo is easy to follow
// -----------------------------------------------------------------------
void DemoStep(const std::string& title) {
    std::cout << "\n========================================\n";
    std::cout << "  DEMO: " << title << "\n";
    std::cout << "========================================\n";
    SDL_Delay(2000); // pause 1 second between steps so changes are visible
}

int main()
{
    // -----------------------------------------------------------------------
    //  DEMO 1 — GameView standalone usage
    //  GameView manages the SDL window and renderer independently.
    // -----------------------------------------------------------------------
    DemoStep("GameView — Standalone Window Creation");

    {
        cse498::GameView view("GameView Standalone Demo", 800, 600);

        // view show helper
        auto ShowFor = [](cse498::GameView& v, int ms) {
            Uint32 start = SDL_GetTicks();
            SDL_Event e;
            while (SDL_GetTicks() - start < static_cast<Uint32>(ms)) {
                while (SDL_PollEvent(&e)) {
                    if (e.type == SDL_QUIT) return;
                }
                v.Present();
                SDL_Delay(16); // ~60fps
            }
        };

        // Initialize creates the SDL window and renderer
        if (!view.Initialize()) {
            std::cerr << "GameView failed to initialize." << std::endl;
            return 1;
        }

        std::cout << "Window title:   Slay The Dungeon\n";
        std::cout << "Window width:   " << view.GetWidth()  << "px\n";
        std::cout << "Window height:  " << view.GetHeight() << "px\n";
        std::cout << "Renderer ready: " << (view.IsReady() ? "yes" : "no") << "\n";
        std::cout << "Window ptr:     " << view.GetWindow()   << "\n";
        std::cout << "Renderer ptr:   " << view.GetRenderer() << "\n";

        // Black screen
        DemoStep("GameView — Clear and Present (black screen)");
        view.Clear();
        ShowFor(view, 2500);

        // GameView::Shutdown destroys window and renderer cleanly
        DemoStep("GameView — Shutdown");
        view.Shutdown();
        std::cout << "IsReady after shutdown: " << (view.IsReady() ? "yes" : "no") << "\n";
    }

    // -----------------------------------------------------------------------
    //  DEMO 2 — ErrorManager GUI popup
    //  ErrorManager can show SDL message box popups for in-game errors.
    // -----------------------------------------------------------------------
    DemoStep("ErrorManager — GUI Error Popup");

    {
        cse498::GameView view("ErrorManager Demo", 800, 600);
        view.Initialize();

        cse498::ErrorManager em;
        em.SetWindow(view.GetWindow());

        std::cout << "Showing a GUI error popup...\n";
        em.GUIError("This is a demo GUI error — click OK to continue.");

        std::cout << "Showing a terminal warning (no popup)...\n";
        em.RaiseTerminalWarning("This warning only appears in the terminal.");

        std::cout << "Demonstrating color customization...\n";
        em.SetTerminalErrorColor(cse498::ErrorManager::ErrorColor::CYAN);
        em.RaiseTerminalError("This error is now cyan instead of red.");

        view.Shutdown();
    }

    // -----------------------------------------------------------------------
    //  DEMO 3 — Game
    //  Game wraps GameView and manages the full game state machine.
    //  States: MAIN_MENU -> OVERWORLD -> DUNGEON -> PAUSED -> SETTINGS
    // -----------------------------------------------------------------------
    DemoStep("Game — Initialization and State Machine");

    {
        cse498::Game game("Slay The Dungeon", 800, 600);

        // Initialize sets up SDL, GameView, menus, worlds, and image assets
        if (!game.Initialize()) {
            std::cerr << "Game failed to initialize." << std::endl;
            return 1;
        }

        // Check initial state
        std::cout << "Initial state: MAIN_MENU ("
                  << (game.GetState() == cse498::GameState::MAIN_MENU ? "confirmed" : "unexpected")
                  << ")\n";

        // GameView is accessible via Game for external SDL operations
        auto view = game.GetGameView();
        std::cout << "Window ready via Game::GetGameView(): "
                  << (view->IsReady() ? "yes" : "no") << "\n";
        std::cout << "Window size: "
                  << view->GetWidth() << "x" << view->GetHeight() << "\n";


        // Launch the full game loop
        DemoStep("Game — Full Run() loop (press ESC to pause once in game)");
        std::cout << "Controls:\n";
        std::cout << "  Arrow keys / Enter — navigate menus\n";
        std::cout << "  New Game           — enter OVERWORLD (grass world)\n";
        std::cout << "  WASD               — scroll camera / move agents one turn\n";
        std::cout << "  ESC                — pause (shows overlay + pause menu)\n";
        std::cout << "  Pause > Dungeon    — enter DUNGEON (BSP generated)\n";
        std::cout << "  Pause > Main Menu  — return to main menu\n";
        std::cout << "  Quit               — exit\n\n";

        game.Run();
    }

    std::cout << "\nDemo complete.\n";
    return 0;
}