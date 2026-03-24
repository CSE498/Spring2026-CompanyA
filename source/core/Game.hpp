/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief Top-level Game class that owns and manages core game systems.
 * @note Status: PROPOSAL
 */

#pragma once

#include <memory>
#include <string>

#include "../Interfaces/GUI/GameView.hpp"
#include "../Interfaces/GUI/Menu.hpp"
#include "../Interfaces/GUI/Text.hpp"
#include "../Interfaces/GUI/ImageManager.hpp"
#include "../Interfaces/GUI/ImageGrid.hpp"
#include "OverWorld.hpp"

namespace cse498 {

  enum class GameState {
    MAIN_MENU,
    OVERWORLD,   ///< Flat interactive world with dungeon entrance
    DUNGEON,     ///< Procedurally generated dungeon world
    PAUSED,      ///< Paused (reachable from OVERWORLD or DUNGEON)
    SETTINGS,    ///< Settings screen (placeholder)
    QUIT
  };

  class Game {
  private:
    std::shared_ptr<GameView> mGameView;
    GameState mState = GameState::MAIN_MENU;
    GameState mPreviousState = GameState::MAIN_MENU; ///< Used to resume after pause

    // Main menu
    Menu mMainMenu;
    Text mTitleText;

    // Pause menu
    Menu mPauseMenu;
    Text mPauseText;

    // Overworld
    std::unique_ptr<ImageManager> mImageManager;
    std::unique_ptr<ImageGrid>    mOverworldGrid;
    std::unique_ptr<OverWorld>    mOverWorld;
    int mCamX = 0; ///< Camera position in tile coordinates
    int mCamY = 0;

    // Dungeon
    std::unique_ptr<ImageGrid> mDungeonGrid;
    int mDungeonCamX = 0; ///< Dungeon camera position in tile coordinates
    int mDungeonCamY = 0;

    bool mRunning = false;
    bool mTurnTaken = false; ///< Set to true when player takes an action, consumed by UpdateOverworld

    // -- Per-state update and render --
    void HandleEvents();

    void UpdateMainMenu();
    void UpdateOverworld();
    void UpdateDungeon();
    void UpdatePaused();
    void UpdateSettings();

    void RenderMainMenu();
    void RenderOverworld();
    void RenderDungeon();
    void RenderPaused();
    void RenderSettings();

    void ProcessPlayerMove(SDL_Keycode key);

    void UpdateWorld(ImageGrid& grid, int& camX, int& camY);
    void RenderWorld(ImageGrid& grid, int camX, int camY);

    // -- Helpers --
    void SetupMainMenu();
    void SetupPauseMenu();
    void SetupOverworld();
    void TransitionTo(GameState new_state);
    void Pause();
    void Resume();

  public:
    Game(const std::string & title = "Slay the Dungeon",
         int width = 800,
         int height = 600)
      : mGameView(std::make_shared<GameView>(title, width, height))
      , mTitleText(nullptr)
      , mPauseText(nullptr)
    { }

    ~Game() = default;

    Game(const Game &) = delete;
    Game & operator=(const Game &) = delete;

    Game(Game &&) = default;
    Game & operator=(Game &&) = default;

    /// Initialize SDL, create the window and renderer, set up menus.
    /// @return true on success, false on failure
    bool Initialize();

    /// Run the main game loop until the window is closed or QUIT state is reached.
    void Run();

    /// Signal the game loop to stop.
    void Quit() { mRunning = false; }

    // -- Accessors --
    [[nodiscard]] std::shared_ptr<GameView> GetGameView() const { return mGameView; }
    [[nodiscard]] GameState GetState() const { return mState; }
  };

} // namespace cse498