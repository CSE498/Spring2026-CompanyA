/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief Implementation of the Game class.
 */

#include "Game.hpp"
#include <SDL2/SDL.h>

namespace cse498 {

  // -----------------------------------------------------------------------
  //  Initialization
  // -----------------------------------------------------------------------

  bool Game::Initialize() {
    if (!mGameView->Initialize()) return false;

    SDL_Renderer* renderer = mGameView->GetRenderer();

    // Title text
    mTitleText.SetRenderer(renderer);
    mTitleText.SetContent("Slay the Dungeon");
    mTitleText.SetSize(48);
    mTitleText.SetBold(true);

    // Pause text
    mPauseText.SetRenderer(renderer);
    mPauseText.SetContent("Paused");
    mPauseText.SetSize(48);
    mPauseText.SetBold(true);

    // Set up image manager
    mImageManager = std::make_unique<ImageManager>(renderer);
    mImageManager->load_image("grass", "assets/tiles/grass.png");

    // Set up overworld — 50x50 tile world, rendered at 64x64 per tile
    mOverworldGrid = std::make_unique<ImageGrid>(50, 50, 64, 64);
    mOverworldGrid->Fill("grass");

    SetupMainMenu();
    SetupPauseMenu();
    return true;
  }

  void Game::SetupMainMenu() {
    mMainMenu.clear();

    mMainMenu.add_option("New Game", [this]() {
      TransitionTo(GameState::OVERWORLD);
    });

    mMainMenu.add_option("Continue", [this]() {
      // TODO: load save state and transition to OVERWORLD or DUNGEON
    });

    mMainMenu.add_option("Settings", [this]() {
      TransitionTo(GameState::SETTINGS);
    });

    mMainMenu.add_option("Quit", [this]() {
      Quit();
    });
  }

  void Game::SetupPauseMenu() {
    mPauseMenu.clear();

    mPauseMenu.add_option("Resume", [this]() {
      Resume();
    });

    mPauseMenu.add_option("Settings", [this]() {
      TransitionTo(GameState::SETTINGS);
    });

    mPauseMenu.add_option("Quit to Main Menu", [this]() {
      TransitionTo(GameState::MAIN_MENU);
    });
  }

  // -----------------------------------------------------------------------
  //  Main Loop
  // -----------------------------------------------------------------------

  void Game::Run() {
    mRunning = true;
    while (mRunning && mState != GameState::QUIT) {
      HandleEvents();

      switch (mState) {
        case GameState::MAIN_MENU: UpdateMainMenu(); break;
        case GameState::OVERWORLD: UpdateOverworld(); break;
        case GameState::DUNGEON:   UpdateDungeon();   break;
        case GameState::PAUSED:    UpdatePaused();    break;
        case GameState::SETTINGS:  UpdateSettings();  break;
        default: break;
      }

      mGameView->Clear();
      switch (mState) {
        case GameState::MAIN_MENU: RenderMainMenu(); break;
        case GameState::OVERWORLD: RenderOverworld(); break;
        case GameState::DUNGEON:   RenderDungeon();   break;
        case GameState::PAUSED:    RenderPaused();    break;
        case GameState::SETTINGS:  RenderSettings();  break;
        default: break;
      }
      mGameView->Present();
    }

    mGameView->Shutdown();
  }

  // -----------------------------------------------------------------------
  //  Event Handling
  // -----------------------------------------------------------------------

  void Game::HandleEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (event.type == SDL_QUIT) {
        mRunning = false;
      }

      if (event.type == SDL_KEYDOWN) {
        switch (event.key.keysym.sym) {

          // Main menu navigation
          case SDLK_UP:
            if (mState == GameState::MAIN_MENU) mMainMenu.select_previous();
            if (mState == GameState::PAUSED)    mPauseMenu.select_previous();
            break;
          case SDLK_DOWN:
            if (mState == GameState::MAIN_MENU) mMainMenu.select_next();
            if (mState == GameState::PAUSED)    mPauseMenu.select_next();
            break;
          case SDLK_RETURN:
            if (mState == GameState::MAIN_MENU) mMainMenu.activate_selected();
            if (mState == GameState::PAUSED)    mPauseMenu.activate_selected();
            break;

          // Pause / resume
          case SDLK_ESCAPE:
            if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON) {
              Pause();
            } else if (mState == GameState::PAUSED) {
              Resume();
            } else if (mState == GameState::SETTINGS) {
              TransitionTo(GameState::MAIN_MENU);
            }
            break;

          default: break;
        }
      }
    }
  }

  // -----------------------------------------------------------------------
  //  State Transitions
  // -----------------------------------------------------------------------

  void Game::TransitionTo(GameState new_state) {
    mPreviousState = mState;
    mState = new_state;
  }

  void Game::Pause() {
    mPreviousState = mState;
    mState = GameState::PAUSED;
    mPauseMenu.select_option(0); // Always start pause menu on "Resume"
  }

  void Game::Resume() {
    mState = mPreviousState;
  }

  // -----------------------------------------------------------------------
  //  Update
  // -----------------------------------------------------------------------

  void Game::UpdateMainMenu() { }

  void Game::UpdateOverworld() {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    int tw = static_cast<int>(mOverworldGrid->GetTileWidth());
    int th = static_cast<int>(mOverworldGrid->GetTileHeight());

    // How many tiles fit on screen
    int tiles_x = mGameView->GetWidth()  / tw;
    int tiles_y = mGameView->GetHeight() / th;

    // Max camera position so the viewport never scrolls past the grid edge
    int max_cam_x = std::max(0, static_cast<int>(mOverworldGrid->GetWidth())  - tiles_x);
    int max_cam_y = std::max(0, static_cast<int>(mOverworldGrid->GetHeight()) - tiles_y);

    if (keys[SDL_SCANCODE_W]) mCamY = std::max(0, mCamY - 1);
    if (keys[SDL_SCANCODE_S]) mCamY = std::min(max_cam_y, mCamY + 1);
    if (keys[SDL_SCANCODE_A]) mCamX = std::max(0, mCamX - 1);
    if (keys[SDL_SCANCODE_D]) mCamX = std::min(max_cam_x, mCamX + 1);
  }

  void Game::UpdateDungeon() {
    // TODO: tick dungeon world logic
  }

  void Game::UpdatePaused() { }

  void Game::UpdateSettings() { }

  // -----------------------------------------------------------------------
  //  Render
  // -----------------------------------------------------------------------

  void Game::RenderMainMenu() {
    SDL_Renderer* renderer = mGameView->GetRenderer();
    int w = mGameView->GetWidth();
    int h = mGameView->GetHeight();

    int menu_w = w / 4;
    int menu_h = static_cast<int>(mMainMenu.get_option_count()) * 50;
    int menu_x = (w - menu_w) / 2;
    int menu_y = (h - menu_h) / 2;

    int title_x = (w - mTitleText.GetWidth()) / 2;
    mTitleText.Draw(title_x, menu_y - 80);

    mMainMenu.draw(renderer, menu_x, menu_y, menu_w, menu_h);
  }

  void Game::RenderOverworld() {
    mOverworldGrid->DrawViewport(
        *mImageManager,
        mCamX, mCamY,
        mGameView->GetWidth(),
        mGameView->GetHeight()
    );
  }

  void Game::RenderDungeon() {
    // TODO: render the dungeon
  }

  void Game::RenderPaused() {
    SDL_Renderer* renderer = mGameView->GetRenderer();
    int w = mGameView->GetWidth();
    int h = mGameView->GetHeight();

    // Semi-transparent dark overlay over whatever world was active
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 160);
    SDL_Rect overlay = {0, 0, w, h};
    SDL_RenderFillRect(renderer, &overlay);

    // "Paused" title
    int pause_x = (w - mPauseText.GetWidth()) / 2;
    int pause_y = h / 4;
    mPauseText.Draw(pause_x, pause_y);

    // Pause menu centered
    int menu_w = w / 4;
    int menu_h = static_cast<int>(mPauseMenu.get_option_count()) * 50;
    int menu_x = (w - menu_w) / 2;
    int menu_y = (h - menu_h) / 2;
    mPauseMenu.draw(renderer, menu_x, menu_y, menu_w, menu_h);
  }

  void Game::RenderSettings() {
    // TODO: render settings screen
  }

} // namespace cse498