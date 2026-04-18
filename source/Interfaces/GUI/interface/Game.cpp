/**
 * This file is part of the Spring 2026, CSE 498, section 2, course project.
 * @brief Implementation of the Game class.
 */

#include "Game.hpp"
#include <SDL2/SDL.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include "../../../core/AgentBase.hpp"

namespace cse498 {

class StubAgent : public AgentBase {
public:
    StubAgent(size_t id, const std::string& name, const WorldBase& world) : AgentBase(id, name, world) {
        SetSymbol('@'); // represents player on screen
    }

    size_t SelectAction(const WorldGrid& /*grid*/) override {
        return 0; // 0 -> remain still
    }

    void Notify(const std::string& message, const std::string& msg_type = "none") override {
        std::cout << "[Agent Notification] type= " << msg_type << " message = " << message << std::endl;
    }
};

// -----------------------------------------------------------------------
//  Initialization
// -----------------------------------------------------------------------

bool Game::Initialize() {
    if (!mGameView->Initialize())
        return false;

    std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;

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

    // Helper lambda to load and propagate errors
    auto LoadCheck = [&](const std::string& name, const std::string& path) -> bool {
        auto result = mImageManager->LoadImage(name, path);
        if (!result) {
            std::cerr << "LoadImage failed: " << result.error() << std::endl;
            return false;
        }
        return true;
    };

    // Grass variants
    if (!LoadCheck("grass", std::string(ASSETS_DIR) + "Tiles/grass.png"))
        return false;
    if (!LoadCheck("grass_flowers", std::string(ASSETS_DIR) + "Tiles/grass_flowers.png"))
        return false;
    if (!LoadCheck("grass_bones", std::string(ASSETS_DIR) + "Tiles/grass_bones.png"))
        return false;
    if (!LoadCheck("grass_mud", std::string(ASSETS_DIR) + "Tiles/grass_mud.png"))
        return false;
    if (!LoadCheck("grass_rock", std::string(ASSETS_DIR) + "Tiles/grass_rock.png"))
        return false;

    // Structure
    if (!LoadCheck("entrance", std::string(ASSETS_DIR) + "Tiles/grass_left_entrance.png"))
        return false;

    // Border walls
    if (!LoadCheck("wall_left", std::string(ASSETS_DIR) + "Tiles/grass_wall_left.png"))
        return false;
    if (!LoadCheck("wall_right", std::string(ASSETS_DIR) + "Tiles/grass_wall_right.png"))
        return false;
    if (!LoadCheck("wall_top", std::string(ASSETS_DIR) + "Tiles/grass_wall_up.png"))
        return false;
    if (!LoadCheck("wall_bottom", std::string(ASSETS_DIR) + "Tiles/grass_wall_bottom.png"))
        return false;
    if (!LoadCheck("wall_corner", std::string(ASSETS_DIR) + "Tiles/grass_wall_up.png"))
        return false;

    // Mobs
    if (!LoadCheck("skeleton", std::string(ASSETS_DIR) + "Mobs/skeleton.png"))
        return false;

    // Dungeon tile images
    if (!LoadCheck("wall", std::string(ASSETS_DIR) + "Tiles/grass.png"))
        return false;
    if (!LoadCheck("floor", std::string(ASSETS_DIR) + "Tiles/stone.png"))
        return false;
    if (!LoadCheck("dot", std::string(ASSETS_DIR) + "Tiles/stone.png"))
        return false;

    // Player
    if (!LoadCheck("player", std::string(ASSETS_DIR) + "Player/player.png"))
        return false;

    // World Setups
    SetupOverworld();
    SetupDungeon();

    // Set up dungeon world — 50x50 tile world, rendered at 64x64 per tile
    //    mDungeonGrid = std::make_unique<ImageGrid>(50, 50, 64, 64);
    //    mDungeonGrid->Fill("stone");

    // Agent module integration demonstration
    StubAgent player(1, "Player", *mOverWorld);
    player.AddAction("up", 1);
    player.AddAction("down", 2);
    player.AddAction("left", 3);
    player.AddAction("right", 4);

    std::cout << "Agent: " << player.GetName() << " (symbol: " << player.GetSymbol() << ")" << std::endl;
    std::cout << "Movement actions registered: " << (player.HasAction("up") ? "yes" : "no") << std::endl;
    player.Notify("Welcome to the overworld!", "system");


    SetupMainMenu();
    SetupPauseMenu();
    return true;
}

void Game::SetupOverworld() {
    mOverWorld = std::make_unique<OverWorld>();
    mOverWorld->AddPacingAgent("skeleton", 2, 2, true);

    const WorldGrid& grid = mOverWorld->GetGrid();
    size_t world_w = grid.GetWidth();
    size_t world_h = grid.GetHeight();

    mOverworldGrid = std::make_unique<ImageGrid>(world_w, world_h, 64, 64);

    // Map every WorldGrid cell type name to its matching image name
    for (size_t y = 0; y < world_h; ++y) {
        for (size_t x = 0; x < world_w; ++x) {
            WorldPosition pos(x, y);
            const std::string& cell_name = grid.GetCellTypeName(grid[pos]);
            mOverworldGrid->SetCell(x, y, cell_name);
        }
    }

    mOverWorld->SetAnalyticsManager(mAnalyticsManager);
}


void Game::SetupDungeon() {
    mDungeonWorld = std::make_unique<DungeonWorld>();

    const WorldGrid& grid = mDungeonWorld->GetGrid();
    size_t world_w = grid.GetWidth();
    size_t world_h = grid.GetHeight();

    mDungeonGrid = std::make_unique<ImageGrid>(world_w, world_h, 64, 64);

    // Map every cell type name to its matching image name
    for (size_t y = 0; y < world_h; ++y) {
        for (size_t x = 0; x < world_w; ++x) {
            WorldPosition pos(x, y);
            const std::string& cell_name = grid.GetCellTypeName(grid[pos]);
            mDungeonGrid->SetCell(x, y, cell_name);
        }
    }

    mDungeonWorld->SetAnalyticsManager(mAnalyticsManager);
}

void Game::SetupMainMenu() {
    mMainMenu.Clear();

    mMainMenu.AddOption("New Game", [this]() { TransitionTo(GameState::OVERWORLD); });

    mMainMenu.AddOption("Settings", [this]() { TransitionTo(GameState::SETTINGS); });

    mMainMenu.AddOption("Quit", [this]() { Quit(); });
}

void Game::SetupPauseMenu() {
    mPauseMenu.Clear();

    mPauseMenu.AddOption("Resume", [this]() { Resume(); });

    mPauseMenu.AddOption("Go to Dungeon World", [this]() {
        TransitionTo(GameState::DUNGEON);
        mPreviousState = GameState::DUNGEON;
    });

    mPauseMenu.AddOption("Go to Overworld", [this]() {
        TransitionTo(GameState::OVERWORLD);
        mPreviousState = GameState::OVERWORLD;
    });

    mPauseMenu.AddOption("Stats", [this]() { TransitionTo(GameState::STATS); });

    mPauseMenu.AddOption("Settings", [this]() { TransitionTo(GameState::SETTINGS); });

    mPauseMenu.AddOption("Quit to Main Menu", [this]() { TransitionTo(GameState::MAIN_MENU); });
}

// -----------------------------------------------------------------------
//  Main Loop
// -----------------------------------------------------------------------

void Game::Run() {
    mRunning = true;
    while (mRunning && mState != GameState::QUIT) {
        HandleEvents();

        switch (mState) {
            case GameState::MAIN_MENU:
                UpdateMainMenu();
                break;
            case GameState::OVERWORLD:
                UpdateOverworld();
                break;
            case GameState::DUNGEON:
                UpdateDungeon();
                break;
            case GameState::PAUSED:
                UpdatePaused();
                break;
            case GameState::STATS:
                UpdateStats();
                break;
            case GameState::SETTINGS:
                UpdateSettings();
                break;
            default:
                break;
        }

        mGameView->Clear();
        switch (mState) {
            case GameState::MAIN_MENU:
                RenderMainMenu();
                break;
            case GameState::OVERWORLD:
                RenderOverworld();
                break;
            case GameState::DUNGEON:
                RenderDungeon();
                break;
            case GameState::PAUSED:
                RenderPaused();
                break;
            case GameState::STATS:
                RenderStats();
                break;
            case GameState::SETTINGS:
                RenderSettings();
                break;
            default:
                break;
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
                    if (mState == GameState::MAIN_MENU)
                        mMainMenu.SelectPrevious();
                    if (mState == GameState::PAUSED)
                        mPauseMenu.SelectPrevious();
                    break;
                case SDLK_DOWN:
                    if (mState == GameState::MAIN_MENU)
                        mMainMenu.SelectNext();
                    if (mState == GameState::PAUSED)
                        mPauseMenu.SelectNext();
                    break;
                case SDLK_RETURN:
                    if (mState == GameState::MAIN_MENU)
                        mMainMenu.ActivateSelected();
                    if (mState == GameState::PAUSED)
                        mPauseMenu.ActivateSelected();
                    break;

                    // Player movement — one turn per keypress with 150ms cooldown
                case SDLK_w:
                case SDLK_s:
                case SDLK_a:
                case SDLK_d:
                    if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON) {
                        static Uint32 last_move_time = 0;
                        Uint32 now = SDL_GetTicks();
                        if (now - last_move_time >= 150) {
                            ProcessPlayerMove(event.key.keysym.sym);
                            last_move_time = now;
                        }
                    }
                    break;

                    // Pause / resume
                case SDLK_ESCAPE:
                    if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON) {
                        Pause();
                    } else if (mState == GameState::PAUSED) {
                        Resume();
                    } else if (mState == GameState::SETTINGS || mState == GameState::STATS) {
                        Resume();
                    }
                    break;

                default:
                    break;
            }
        }
    }
}

// -----------------------------------------------------------------------
//  State Transitions
// -----------------------------------------------------------------------

void Game::TransitionTo(GameState new_state) {
    if ((mPreviousState == GameState::DUNGEON || mPreviousState == GameState::OVERWORLD) &&
        new_state == GameState::MAIN_MENU) {
        mAnalyticsManager->LogDamageDealt(mAnalyticsManager->GetCurrentRunStats().damageDealt);
        mAnalyticsManager->LogEnemiesKilled(mAnalyticsManager->GetCurrentRunStats().enemiesKilled);
        mAnalyticsManager->ResetCurrentRunStats();
    }
    mPreviousState = mState;
    mState = new_state;
}

void Game::Pause() {
    mPreviousState = mState;
    mState = GameState::PAUSED;
    mPauseMenu.SelectOption(0); // Always start pause menu on "Resume"
}

void Game::Resume() { mState = mPreviousState; }

// -----------------------------------------------------------------------
//  Update
// -----------------------------------------------------------------------

void Game::UpdateMainMenu() {}

void Game::UpdateOverworld() {
    if (mTurnTaken) {
        mOverWorld->RunAgents();
        mTurnTaken = false;
    }
}
void Game::UpdateDungeon() {}

void Game::UpdateWorld(ImageGrid& grid, int& camX, int& camY) {
    const Uint8* keys = SDL_GetKeyboardState(nullptr);

    // Only move once every 150ms TODO
    static Uint32 last_move_time = 0;
    Uint32 now = SDL_GetTicks();
    if (now - last_move_time < 150)
        return;

    int tw = static_cast<int>(grid.GetTileWidth());
    int th = static_cast<int>(grid.GetTileHeight());

    // How many Tiles fit on screen
    int Tiles_x = mGameView->GetWidth() / tw;
    int Tiles_y = mGameView->GetHeight() / th;

    // Max camera position so the viewport never scrolls past the grid edge
    int max_cam_x = std::max(0, static_cast<int>(grid.GetWidth()) - Tiles_x);
    int max_cam_y = std::max(0, static_cast<int>(grid.GetHeight()) - Tiles_y);

    bool moved = false;
    if (keys[SDL_SCANCODE_W]) {
        camY = std::max(0, camY - 1);
        moved = true;
    }
    if (keys[SDL_SCANCODE_S]) {
        camY = std::min(max_cam_y, camY + 1);
        moved = true;
    }
    if (keys[SDL_SCANCODE_A]) {
        camX = std::max(0, camX - 1);
        moved = true;
    }
    if (keys[SDL_SCANCODE_D]) {
        camX = std::min(max_cam_x, camX + 1);
        moved = true;
    }

    if (moved)
        last_move_time = now;
}

void Game::UpdatePaused() {}
void Game::UpdateSettings() {}

// -----------------------------------------------------------------------
//  Render
// -----------------------------------------------------------------------

void Game::RenderMainMenu() {

    int w = mGameView->GetWidth();
    int h = mGameView->GetHeight();

    int menu_w = w / 4;
    int menu_h = static_cast<int>(mMainMenu.GetOptionCount()) * 50;
    int menu_x = (w - menu_w) / 2;
    int menu_y = (h - menu_h) / 2;

    int title_x = (w - mTitleText.GetWidth()) / 2;
    mTitleText.Draw(title_x, menu_y - 80);

    SDL_Renderer* renderer = mGameView->GetRenderer();
    mMainMenu.DrawMenu(renderer, menu_x, menu_y, menu_w, menu_h);
}

// Z-layer ordering. Put here for future reference of probable Game draw logic
void Game::RenderOverworld() {
    // Layer 0 — Tiles (bottom)
    RenderWorld(*mOverworldGrid, mCamX, mCamY);

    // Layer 1 — items/objects on the ground
    // RenderItems();

    // Layer 2 — agents/NPCs
    int tw = static_cast<int>(mOverworldGrid->GetTileWidth());
    int th = static_cast<int>(mOverworldGrid->GetTileHeight());

    for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
        const AgentBase& agent = mOverWorld->GetAgent(i);
        const WorldPosition& pos = agent.GetLocation().AsWorldPosition();

        int screen_x = (static_cast<int>(pos.CellX()) - mCamX) * tw;
        int screen_y = (static_cast<int>(pos.CellY()) - mCamY) * th;

        mImageManager->DrawImage(mOverWorld->GetAgentSpriteName(), screen_x, screen_y, tw, th);
    }

    // Layer 3 — dummy player

    int player_screen_x = (mPlayerX - mCamX) * tw;
    int player_screen_y = (mPlayerY - mCamY) * th;

    mImageManager->DrawImage("player", player_screen_x, player_screen_y, tw, th);

    // Layer 4 — UI/HUD (health bar, etc.)
    // RenderHUD();
}

void Game::RenderDungeon() {
    RenderWorld(*mDungeonGrid, mDungeonCamX, mDungeonCamY);

    int tw = static_cast<int>(mDungeonGrid->GetTileWidth());
    int th = static_cast<int>(mDungeonGrid->GetTileHeight());

    int player_screen_x = (mDungeonPlayerX - mDungeonCamX) * tw;
    int player_screen_y = (mDungeonPlayerY - mDungeonCamY) * th;

    mImageManager->DrawImage("player", player_screen_x, player_screen_y, tw, th);
}

void Game::RenderWorld(const ImageGrid& grid, int camX, int camY) {
    grid.DrawViewport(*mImageManager, camX, camY, mGameView->GetWidth(), mGameView->GetHeight());
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
    int menu_h = static_cast<int>(mPauseMenu.GetOptionCount()) * 50;
    int menu_x = (w - menu_w) / 2;
    int menu_y = pause_y + mPauseText.GetHeight() + (h / 30);
    mPauseMenu.DrawMenu(renderer, menu_x, menu_y, menu_w, menu_h);
}

void Game::RenderSettings() {
    // TODO: render settings screen
}

void Game::UpdateStats() {}

void Game::RenderStats() {
    SDL_Renderer* renderer = mGameView->GetRenderer();
    int w = mGameView->GetWidth();
    int h = mGameView->GetHeight();

    // Dark background
    SDL_SetRenderDrawColor(renderer, 20, 20, 30, 255);
    SDL_Rect bg = {0, 0, w, h};
    SDL_RenderFillRect(renderer, &bg);
}

void Game::ProcessPlayerMove(SDL_Keycode key) {
    if (mState == GameState::OVERWORLD) {
        int max_x = static_cast<int>(mOverworldGrid->GetWidth()) - 1;
        int max_y = static_cast<int>(mOverworldGrid->GetHeight()) - 1;

        switch (key) {
            case SDLK_w:
                mPlayerY = std::max(0, mPlayerY - 1);
                break;
            case SDLK_s:
                mPlayerY = std::min(max_y, mPlayerY + 1);
                break;
            case SDLK_a:
                mPlayerX = std::max(0, mPlayerX - 1);
                break;
            case SDLK_d:
                mPlayerX = std::min(max_x, mPlayerX + 1);
                break;
            default:
                break;
        }

        int tw = static_cast<int>(mOverworldGrid->GetTileWidth());
        int th = static_cast<int>(mOverworldGrid->GetTileHeight());

        int Tiles_x = mGameView->GetWidth() / tw;
        int Tiles_y = mGameView->GetHeight() / th;

        int max_cam_x = std::max(0, static_cast<int>(mOverworldGrid->GetWidth()) - Tiles_x);
        int max_cam_y = std::max(0, static_cast<int>(mOverworldGrid->GetHeight()) - Tiles_y);

        mCamX = std::clamp(mPlayerX - Tiles_x / 2, 0, max_cam_x);
        mCamY = std::clamp(mPlayerY - Tiles_y / 2, 0, max_cam_y);
    }

    else if (mState == GameState::DUNGEON) {
        int max_x = static_cast<int>(mDungeonGrid->GetWidth()) - 1;
        int max_y = static_cast<int>(mDungeonGrid->GetHeight()) - 1;

        switch (key) {
            case SDLK_w:
                mDungeonPlayerY = std::max(0, mDungeonPlayerY - 1);
                break;
            case SDLK_s:
                mDungeonPlayerY = std::min(max_y, mDungeonPlayerY + 1);
                break;
            case SDLK_a:
                mDungeonPlayerX = std::max(0, mDungeonPlayerX - 1);
                break;
            case SDLK_d:
                mDungeonPlayerX = std::min(max_x, mDungeonPlayerX + 1);
                break;
            default:
                break;
        }

        int tw = static_cast<int>(mDungeonGrid->GetTileWidth());
        int th = static_cast<int>(mDungeonGrid->GetTileHeight());

        int Tiles_x = mGameView->GetWidth() / tw;
        int Tiles_y = mGameView->GetHeight() / th;

        int max_cam_x = std::max(0, static_cast<int>(mDungeonGrid->GetWidth()) - Tiles_x);
        int max_cam_y = std::max(0, static_cast<int>(mDungeonGrid->GetHeight()) - Tiles_y);

        mDungeonCamX = std::clamp(mDungeonPlayerX - Tiles_x / 2, 0, max_cam_x);
        mDungeonCamY = std::clamp(mDungeonPlayerY - Tiles_y / 2, 0, max_cam_y);
    }

    mTurnTaken = true;
}

} // namespace cse498
