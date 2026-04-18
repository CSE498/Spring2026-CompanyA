/**
 * @file GameView.h
 * @author Group 17
 *
 * Manages the SDL window and renderer for the game.
 * Made via reference to Claude Sonnet 4.6
 */

#pragma once

#include <SDL2/SDL.h>
#include <string>

namespace cse498 {

class GameView {
private:
    SDL_Window* mWindow = nullptr;
    SDL_Renderer* mRenderer = nullptr;

    std::string mTitle;
    int mWidth;
    int mHeight;

    static constexpr SDL_Color mBackgroundColor = {30, 30, 30, 255};

public:
    GameView(const std::string& title = "Slay the Dungeon", int width = 800, int height = 600);

    ~GameView();

    // No copy
    GameView(const GameView&) = delete;
    GameView& operator=(const GameView&) = delete;

    /**
     * Initialize SDL, create window and renderer.
     * @return true on success, false on failure
     */
    bool Initialize();

    /**
     * Clear the screen to a solid color.
     */
    void Clear();

    /**
     * Present the rendered frame to the screen.
     */
    void Present();

    /**
     * Shutdown SDL and destroy window/renderer.
     */
    void Shutdown();

    /**
     * @return true if the window and renderer are ready to use
     */
    bool IsReady() const;

    // Accessors
    SDL_Window* GetWindow() const { return mWindow; }
    SDL_Renderer* GetRenderer() const { return mRenderer; }
    int GetWidth() const { return mWidth; }
    int GetHeight() const { return mHeight; }
};

} // namespace cse498
