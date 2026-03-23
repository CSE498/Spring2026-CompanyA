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

    // Set up image manager and load all tile assets
    mImageManager = std::make_unique<ImageManager>(renderer);

    // Grass variants
    mImageManager->LoadImage("grass",         "assets/tiles/grass.png");
    mImageManager->LoadImage("grass_flowers", "assets/tiles/grass_flowers.png");
    mImageManager->LoadImage("grass_bones",   "assets/tiles/grass_bones.png");
    mImageManager->LoadImage("grass_mud",     "assets/tiles/grass_mud.png");
    mImageManager->LoadImage("grass_rock",    "assets/tiles/grass_rock.png");

    // Structure
    mImageManager->LoadImage("entrance",      "assets/tiles/grass_left_entrance.png");

    // Border walls
    mImageManager->LoadImage("wall_left",     "assets/tiles/grass_wall_left.png");
    mImageManager->LoadImage("wall_right",    "assets/tiles/grass_wall_right.png");
    mImageManager->LoadImage("wall_top",      "assets/tiles/grass_wall_up.png");
    mImageManager->LoadImage("wall_bottom",   "assets/tiles/grass_wall_bottom.png");
    mImageManager->LoadImage("wall_corner",   "assets/tiles/grass_wall_up.png");

    // Dungeon
    mImageManager->LoadImage("stone",         "assets/tiles/stone.png");

    SetupOverworld();

    // Set up dungeon world — 50x50 tile world, rendered at 64x64 per tile
    mDungeonGrid = std::make_unique<ImageGrid>(50, 50, 64, 64);
    mDungeonGrid->Fill("stone");

    SetupMainMenu();
    SetupPauseMenu();
    return true;
  }

  void Game::SetupOverworld() {
    mOverWorld = std::make_unique<OverWorld>();

    const WorldGrid & grid = mOverWorld->GetGrid();
    size_t world_w = grid.GetWidth();
    size_t world_h = grid.GetHeight();

    mOverworldGrid = std::make_unique<ImageGrid>(world_w, world_h, 64, 64);

    // Map every WorldGrid cell type name to its matching image name
    for (size_t y = 0; y < world_h; ++y) {
      for (size_t x = 0; x < world_w; ++x) {
        WorldPosition pos(x, y);
        const std::string & cell_name = grid.GetCellTypeName(grid[pos]);
        mOverworldGrid->SetCell(x, y, cell_name);
      }
    }
  }

  void Game::SetupMainMenu() {
    mMainMenu.clear();

    mMainMenu.add_option("New Game", [this]() {
      TransitionTo(GameState::OVERWORLD);
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

    mPauseMenu.add_option("Go to Dungeon World", [this]() {
      TransitionTo(GameState::DUNGEON);
      mPreviousState = GameState::DUNGEON;
    });

    mPauseMenu.add_option("Go to Overworld", [this]() {
      TransitionTo(GameState::OVERWORLD);
      mPreviousState = GameState::OVERWORLD;
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

          // Navigation in menus
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
              Resume();
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

  void Game::UpdateOverworld() { UpdateWorld(*mOverworldGrid, mCamX, mCamY); }
  void Game::UpdateDungeon()   { UpdateWorld(*mDungeonGrid, mDungeonCamX, mDungeonCamY); }

  void Game::UpdateWorld(ImageGrid& grid, int& camX, int& camY) {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    // Only move once every 150ms TODO
    static Uint32 last_move_time = 0;
    Uint32 now = SDL_GetTicks();
    if (now - last_move_time < 150) return;

    int tw = static_cast<int>(grid.GetTileWidth());
    int th = static_cast<int>(grid.GetTileHeight());

    // How many tiles fit on screen
    int tiles_x = mGameView->GetWidth()  / tw;
    int tiles_y = mGameView->GetHeight() / th;

    // Max camera position so the viewport never scrolls past the grid edge
    int max_cam_x = std::max(0, static_cast<int>(grid.GetWidth())  - tiles_x);
    int max_cam_y = std::max(0, static_cast<int>(grid.GetHeight()) - tiles_y);

    bool moved = false;
    if (keys[SDL_SCANCODE_W]) { camY = std::max(0, camY - 1);          moved = true; }
    if (keys[SDL_SCANCODE_S]) { camY = std::min(max_cam_y, camY + 1);  moved = true; }
    if (keys[SDL_SCANCODE_A]) { camX = std::max(0, camX - 1);          moved = true; }
    if (keys[SDL_SCANCODE_D]) { camX = std::min(max_cam_x, camX + 1);  moved = true; }

    if (moved) last_move_time = now;
  }

  void Game::UpdatePaused()   { }
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

  void Game::RenderOverworld() { RenderWorld(*mOverworldGrid, mCamX, mCamY); }
  void Game::RenderDungeon()   { RenderWorld(*mDungeonGrid, mDungeonCamX, mDungeonCamY); }

  void Game::RenderWorld(ImageGrid& grid, int camX, int camY) {
    grid.DrawViewport(
        *mImageManager,
        camX, camY,
        mGameView->GetWidth(),
        mGameView->GetHeight()
    );
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
    int menu_y = pause_y + mPauseText.GetHeight() + (h / 30);
    mPauseMenu.draw(renderer, menu_x, menu_y, menu_w, menu_h);
  }

  void Game::RenderSettings() {
    // TODO: render settings screen
  }

} // namespace cse498