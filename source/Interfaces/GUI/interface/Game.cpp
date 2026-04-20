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

namespace cse498
{

    constexpr int TURN_DELAY = 100;

    // -----------------------------------------------------------------------
    //  Initialization
    // -----------------------------------------------------------------------

    bool Game::Initialize()
    {
        if (!mGameView->Initialize())
            return false;

        std::cout << "Working directory: " << std::filesystem::current_path() << std::endl;
        std::cout << "Asset Dir: " << std::string(ASSETS_DIR) << std::endl;

        SDL_Renderer *renderer = mGameView->GetRenderer();

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

        // Item pickup notifications
        mPickupText.SetRenderer(renderer);
        mPickupText.SetSize(20);

        // Set up image manager and load all tile assets
        mImageManager = std::make_unique<ImageManager>(renderer);

        // Helper lambda to load and propagate errors
        auto LoadCheck = [&](const std::string &name, const std::string &path) -> bool
        {
            auto result = mImageManager->LoadImage(name, path);
            if (!result)
            {
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

        // --- Level 1 floors (forest) ---
        if (!LoadCheck("floor_l1v1", std::string(ASSETS_DIR) + "DungeonWorlds/forest/floor_tiles/tile_grass_1.png")) return false;
        if (!LoadCheck("floor_l1v2", std::string(ASSETS_DIR) + "DungeonWorlds/forest/floor_tiles/tile_grass_2.png")) return false;
        if (!LoadCheck("floor_l1v3", std::string(ASSETS_DIR) + "DungeonWorlds/forest/floor_tiles/tile_grass_3.png")) return false;
        if (!LoadCheck("floor_l1v4", std::string(ASSETS_DIR) + "DungeonWorlds/forest/floor_tiles/tile_grass_4.png")) return false;
        if (!LoadCheck("floor_l1v5", std::string(ASSETS_DIR) + "DungeonWorlds/forest/floor_tiles/tile_grass_5.png")) return false;
        // --- Level 2 floors (cave) ---
        if (!LoadCheck("floor_l2v1", std::string(ASSETS_DIR) + "DungeonWorlds/cave/floor_tiles/tile_cave_1.png")) return false;
        if (!LoadCheck("floor_l2v2", std::string(ASSETS_DIR) + "DungeonWorlds/cave/floor_tiles/tile_cave_2.png")) return false;
        if (!LoadCheck("floor_l2v3", std::string(ASSETS_DIR) + "DungeonWorlds/cave/floor_tiles/tile_cave_3.png")) return false;
        if (!LoadCheck("floor_l2v4", std::string(ASSETS_DIR) + "DungeonWorlds/cave/floor_tiles/tile_cave_4.png")) return false;
        if (!LoadCheck("floor_l2v5", std::string(ASSETS_DIR) + "DungeonWorlds/cave/floor_tiles/tile_cave_5.png")) return false;
        // --- Level 3 floors (dungeon) ---
        if (!LoadCheck("floor_l3v1", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/floor_tiles/tile_stoneBrick_1.png")) return false;
        if (!LoadCheck("floor_l3v2", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/floor_tiles/tile_stoneBrick_2.png")) return false;
        if (!LoadCheck("floor_l3v3", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/floor_tiles/tile_stoneBrick_3.png")) return false;
        if (!LoadCheck("floor_l3v4", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/floor_tiles/tile_stoneBrick_4.png")) return false;
        if (!LoadCheck("floor_l3v5", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/floor_tiles/tile_stoneBrick_5.png")) return false;
        // --- Level 4 floors (castle) ---
        if (!LoadCheck("floor_l4v1", std::string(ASSETS_DIR) + "DungeonWorlds/castle/floor_tiles/tile_wood_1.png")) return false;
        if (!LoadCheck("floor_l4v2", std::string(ASSETS_DIR) + "DungeonWorlds/castle/floor_tiles/tile_wood_2.png")) return false;
        if (!LoadCheck("floor_l4v3", std::string(ASSETS_DIR) + "DungeonWorlds/castle/floor_tiles/tile_wood_3.png")) return false;
        if (!LoadCheck("floor_l4v4", std::string(ASSETS_DIR) + "DungeonWorlds/castle/floor_tiles/tile_wood_4.png")) return false;
        if (!LoadCheck("floor_l4v5", std::string(ASSETS_DIR) + "DungeonWorlds/castle/floor_tiles/tile_wood_5.png")) return false;

        // --- Generic wall (#) ---
        if (!LoadCheck("wall", std::string(ASSETS_DIR) + "Tiles/black_tile.png")) return false;

        // --- Level 1 walls (forest) ---
        if (!LoadCheck("wall_l1v1", std::string(ASSETS_DIR) + "DungeonWorlds/forest/walls/external/border_top_forest.png")) return false;
        if (!LoadCheck("wall_l1v2", std::string(ASSETS_DIR) + "DungeonWorlds/forest/walls/external/border_bottom_forest.png")) return false;
        if (!LoadCheck("wall_l1v13", std::string(ASSETS_DIR) + "DungeonWorlds/forest/walls/external/border_left_forest.png")) return false;
        if (!LoadCheck("wall_l1v4", std::string(ASSETS_DIR) + "DungeonWorlds/forest/walls/external/border_right_forest.png")) return false;
        if (!LoadCheck("wall_l1v5", std::string(ASSETS_DIR) + "DungeonWorlds/forest/walls/external/border_top_forest.png")) return false;
        if (!LoadCheck("wall_l1v6", std::string(ASSETS_DIR) + "DungeonWorlds/forest/walls/external/border_top_forest.png")) return false;
        if (!LoadCheck("wall_l1v7", std::string(ASSETS_DIR) + "DungeonWorlds/forest/walls/external/door_left_forest.png")) return false;
        if (!LoadCheck("wall_l1v8", std::string(ASSETS_DIR) + "DungeonWorlds/forest/walls/external/door_right_forest.png")) return false;
        // --- Level 2 walls (cave) ---
        if (!LoadCheck("wall_l2v1", std::string(ASSETS_DIR) + "DungeonWorlds/cave/walls/external/border_top_cave.png")) return false;
        if (!LoadCheck("wall_l2v2", std::string(ASSETS_DIR) + "DungeonWorlds/cave/walls/external/border_bottom_cave.png")) return false;
        if (!LoadCheck("wall_l2v3", std::string(ASSETS_DIR) + "DungeonWorlds/cave/walls/external/border_left_cave.png")) return false;
        if (!LoadCheck("wall_l2v4", std::string(ASSETS_DIR) + "DungeonWorlds/cave/walls/external/border_right_cave.png")) return false;
        if (!LoadCheck("wall_l2v5", std::string(ASSETS_DIR) + "DungeonWorlds/cave/walls/external/border_top_cave.png")) return false;
        if (!LoadCheck("wall_l2v6", std::string(ASSETS_DIR) + "DungeonWorlds/cave/walls/external/border_top_cave.png")) return false;
        if (!LoadCheck("wall_l2v7", std::string(ASSETS_DIR) + "DungeonWorlds/cave/walls/external/door_left_cave.png")) return false;
        if (!LoadCheck("wall_l2v8", std::string(ASSETS_DIR) + "DungeonWorlds/cave/walls/external/door_right_cave.png")) return false;
        // --- Level 3 walls (dungeon) ---
        if (!LoadCheck("wall_l3v1", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/border_top_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v2", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/border_bottom_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v3", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/border_left_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v4", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/border_right_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v5", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/border_top_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v6", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/border_top_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v7", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/door_left_dungeon.png")) return false;
        if (!LoadCheck("wall_l3v8", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/door_right_dungeon.png")) return false;
        // --- Level 4 walls (castle) ---
        if (!LoadCheck("wall_l4v1", std::string(ASSETS_DIR) + "DungeonWorlds/castle/walls/external/border_top_castle.png")) return false;
        if (!LoadCheck("wall_l4v2", std::string(ASSETS_DIR) + "DungeonWorlds/castle/walls/external/border_bottom_castle.png")) return false;
        if (!LoadCheck("wall_l4v3", std::string(ASSETS_DIR) + "DungeonWorlds/castle/walls/external/border_left_castle.png")) return false;
        if (!LoadCheck("wall_l4v4", std::string(ASSETS_DIR) + "DungeonWorlds/castle/walls/external/border_right_castle.png")) return false;
        if (!LoadCheck("wall_l4v5", std::string(ASSETS_DIR) + "DungeonWorlds/castle/walls/external/border_top_castle.png")) return false;
        if (!LoadCheck("wall_l4v6", std::string(ASSETS_DIR) + "DungeonWorlds/castle/walls/external/border_top_castle.png")) return false;
        if (!LoadCheck("wall_l4v7", std::string(ASSETS_DIR) + "DungeonWorlds/castle/walls/external/door_left_castle.png")) return false;
        if (!LoadCheck("wall_l4v8", std::string(ASSETS_DIR) + "DungeonWorlds/castle/walls/external/door_right_castle.png")) return false;

        // --- Shared special tiles ---
        if (!LoadCheck("wall_trap", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/floor_tiles/tile_stoneBrick_3.png")) return false;
        if (!LoadCheck("wall_loot", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_potion_defense.png")) return false;
        if (!LoadCheck("wall_skeleton", std::string(ASSETS_DIR) + "Mobs/skeleton.png")) return false;
        if (!LoadCheck("wall_goblin", std::string(ASSETS_DIR) + "Mobs/goblin.png")) return false;
        if (!LoadCheck("wall_secret", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/door_right_dungeon.png")) return false;
        if (!LoadCheck("exit", std::string(ASSETS_DIR) + "DungeonWorlds/dungeon/walls/external/door_left_dungeon.png")) return false;

        // Item sprites — keyed by item name to match what Inventory stores
        if (!LoadCheck("Sword", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +1", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +2", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +3", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +4", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_sword_1.png")) return false;
        if (!LoadCheck("Sword +5", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_sword_1.png")) return false;

        if (!LoadCheck("Bow", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +1", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +2", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +3", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +4", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_bow_1.png")) return false;
        if (!LoadCheck("Bow +5", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_bow_1.png")) return false;

        if (!LoadCheck("Healing Potion", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_potion_healing.png")) return false;
        if (!LoadCheck("Defense Potion", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_potion_defense.png")) return false;
        if (!LoadCheck("Speed Potion", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_potion_speed.png")) return false;

        if (!LoadCheck("Axe", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_axe.png")) return false;
        if (!LoadCheck("Pickaxe", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_pickaxe.png")) return false;
        if (!LoadCheck("Shovel", std::string(ASSETS_DIR) + "DungeonWorlds/items/item_shovel.png")) return false;

        // Player
        if (!LoadCheck("player", std::string(ASSETS_DIR) + "Player/player.png"))
            return false;

        // UI
        if (!LoadCheck("inventory_bar", std::string(ASSETS_DIR) + "/Player/inventory_bar.png"))
            return false;

        // World Setups
        SetupOverworld();
        SetupDungeon();

        SetupMainMenu();
        SetupPauseMenu();
        return true;
    }

    void Game::SetupOverworld()
    {
        mOverWorld = std::make_unique<OverWorld>();
        mOverWorld->AddPacingAgent("skeleton", 2, 2, true);

        // add a player to the world (based on discord discussion)
        auto& player = mOverWorld->AddAgent<PlayerAgent>("Player");
        player.SetLocation(WorldPosition{1, 1});
        mOverworldPlayer = &player;

        const WorldGrid &grid = mOverWorld->GetGrid();
        size_t world_w = grid.GetWidth();
        size_t world_h = grid.GetHeight();

        mOverworldGrid = std::make_unique<ImageGrid>(world_w, world_h, 64, 64);

        // Map every WorldGrid cell type name to its matching image name
        for (size_t y = 0; y < world_h; ++y)
        {
            for (size_t x = 0; x < world_w; ++x)
            {
                WorldPosition pos(x, y);
                const std::string &cell_name = grid.GetCellTypeName(grid[pos]);
                mOverworldGrid->SetCell(x, y, cell_name);
            }
        }
    }


    void Game::SetupDungeon()
    {
        mDungeonWorld = std::make_unique<DungeonWorld>();

        const WorldGrid &grid = mDungeonWorld->GetGrid();
        size_t world_w = grid.GetWidth();
        size_t world_h = grid.GetHeight();

        mDungeonGrid = std::make_unique<ImageGrid>(world_w, world_h, 64, 64);


        // add a player to the world (based on discord discussion)
        auto& player = mDungeonWorld->AddAgent<PlayerAgent>("Player");
        player.SetLocation(WorldPosition{1, 1});
        mDungeonPlayer = &player;

        std::cout << "Dungeon player ID: " << mDungeonPlayer->GetID() << std::endl;

        // Map every cell type name to its matching image name
        for (size_t y = 0; y < world_h; ++y)
        {
            for (size_t x = 0; x < world_w; ++x)
            {
                WorldPosition pos(x, y);
                const std::string &cell_name = grid.GetCellTypeName(grid[pos]);
                mDungeonGrid->SetCell(x, y, cell_name);
            }
        }
    }

    void Game::SetupMainMenu()
    {
        mMainMenu.Clear();

        mMainMenu.AddOption("New Game", [this]() { TransitionTo(GameState::OVERWORLD); });

        mMainMenu.AddOption("Settings", [this]() { TransitionTo(GameState::SETTINGS); });

        mMainMenu.AddOption("Quit", [this]() { Quit(); });
    }

    void Game::SetupPauseMenu()
    {
        mPauseMenu.Clear();

        mPauseMenu.AddOption("Resume", [this]() { Resume(); });

        mPauseMenu.AddOption("Go to Dungeon World",
                             [this]()
                             {
                                 TransitionTo(GameState::DUNGEON);
                                 mPreviousState = GameState::DUNGEON;
                             });

        mPauseMenu.AddOption("Go to Overworld",
                             [this]()
                             {
                                 TransitionTo(GameState::OVERWORLD);
                                 mPreviousState = GameState::OVERWORLD;
                             });

        mPauseMenu.AddOption("Settings", [this]() { TransitionTo(GameState::SETTINGS); });

        mPauseMenu.AddOption("Quit to Main Menu", [this]() { TransitionTo(GameState::MAIN_MENU); });
    }

    // -----------------------------------------------------------------------
    //  Main Loop
    // -----------------------------------------------------------------------

    void Game::Run()
    {
        mRunning = true;
        while (mRunning && mState != GameState::QUIT)
        {
            HandleEvents();

            switch (mState)
            {
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
            case GameState::SETTINGS:
                UpdateSettings();
                break;
            default:
                break;
            }

            mGameView->Clear();
            switch (mState)
            {
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

    void Game::HandleEvents()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                mRunning = false;
            }

            if (event.type == SDL_KEYDOWN)
            {
                switch (event.key.keysym.sym)
                {

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

                case SDLK_w:
                case SDLK_s:
                case SDLK_a:
                case SDLK_d:
                    if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON)
                    {
                        if (mShowBackpack) {
                            // Navigate backpack grid
                            int rows = static_cast<int>(Inventory::BACKPACK_SIZE / Inventory::ITEMS_PER_ROW);
                            int cols = static_cast<int>(Inventory::ITEMS_PER_ROW);
                            switch (event.key.keysym.sym) {
                            case SDLK_w: mBackpackCursorRow = (mBackpackCursorRow - 1 + rows) % rows; break;
                            case SDLK_s: mBackpackCursorRow = (mBackpackCursorRow + 1) % rows; break;
                            case SDLK_a: mBackpackCursorCol = (mBackpackCursorCol - 1 + cols) % cols; break;
                            case SDLK_d: mBackpackCursorCol = (mBackpackCursorCol + 1) % cols; break;
                            default: break;
                            }
                        } else {
                            static Uint32 last_move_time = 0;
                            Uint32 now = SDL_GetTicks();
                            if (now - last_move_time >= TURN_DELAY)
                            {
                                ProcessPlayerMove(event.key.keysym.sym);
                                last_move_time = now;
                            }
                        }
                    }
                    break;

                case SDLK_TAB:
                    if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON)
                    {
                        mShowBackpack = !mShowBackpack;
                    }
                    break;

                // Number keys 0-9: move backpack item to hotbar slot
                case SDLK_0:
                case SDLK_1:
                case SDLK_2:
                case SDLK_3:
                case SDLK_4:
                case SDLK_5:
                case SDLK_6:
                case SDLK_7:
                case SDLK_8:
                case SDLK_9:
                    if (mShowBackpack && (mState == GameState::OVERWORLD || mState == GameState::DUNGEON))
                    {
                        // SDLK_1 = slot 0, SDLK_2 = slot 1, etc.
                        size_t hotbar_slot = (event.key.keysym.sym == SDLK_0) ? 9 : static_cast<size_t>(event.key.keysym.sym - SDLK_1);
                        if (hotbar_slot < Inventory::HOTBAR_SIZE) {
                            size_t backpack_index = Inventory::HOTBAR_SIZE
                                + mBackpackCursorRow * static_cast<int>(Inventory::ITEMS_PER_ROW)
                                + mBackpackCursorCol;

                            Inventory& inv = (mState == GameState::OVERWORLD)
                                ? mOverworldPlayer->GetInventory()
                                : mDungeonPlayer->GetInventory();

                            inv.SwapSlots(backpack_index, hotbar_slot);
                        }
                    }
                    break;

                    // Pause / resume
                case SDLK_ESCAPE:
                    if (mState == GameState::OVERWORLD || mState == GameState::DUNGEON)
                    {
                        Pause();
                    }
                    else if (mState == GameState::PAUSED)
                    {
                        Resume();
                    }
                    else if (mState == GameState::SETTINGS)
                    {
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

    void Game::TransitionTo(GameState new_state)
    {
        mPreviousState = mState;
        mState = new_state;
    }

    void Game::Pause()
    {
        mPreviousState = mState;
        mState = GameState::PAUSED;
        mPauseMenu.SelectOption(0); // Always start pause menu on "Resume"
    }

    void Game::Resume() { mState = mPreviousState; }

    // -----------------------------------------------------------------------
    //  Update
    // -----------------------------------------------------------------------

    void Game::UpdateMainMenu() {}

    void Game::UpdateOverworld()
    {
        // skip the player in the world agent list, they should choose their own move when needed to.
        if (mTurnTaken) {
            for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i) {
                AgentBase& agent = mOverWorld->GetAgent(i);
                if (&agent == mOverworldPlayer) continue;

                size_t action = agent.SelectAction(mOverWorld->GetGrid());
                mOverWorld->DoAction(agent, action);
            }
            mTurnTaken = false;
        }
    }
    void Game::UpdateDungeon()
    {
        // skip the player in the world agent list, they should choose their own move when needed to.
        if (mTurnTaken) {
            for (size_t i = 0; i < mDungeonWorld->GetNumAgents(); ++i) {
                AgentBase& agent = mDungeonWorld->GetAgent(i);
                if (&agent == mDungeonPlayer) continue;

                size_t action = agent.SelectAction(mDungeonWorld->GetGrid());
                mDungeonWorld->DoAction(agent, action);
            }
            mTurnTaken = false;
        }
    }

    void Game::UpdateWorld(ImageGrid &grid, int &camX, int &camY)
    {
        const Uint8 *keys = SDL_GetKeyboardState(nullptr);

        // Only move once every TURN_DELAY 100ms
        static Uint32 last_move_time = 0;
        Uint32 now = SDL_GetTicks();
        if (now - last_move_time < TURN_DELAY)
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
        if (keys[SDL_SCANCODE_W])
        {
            camY = std::max(0, camY - 1);
            moved = true;
        }
        if (keys[SDL_SCANCODE_S])
        {
            camY = std::min(max_cam_y, camY + 1);
            moved = true;
        }
        if (keys[SDL_SCANCODE_A])
        {
            camX = std::max(0, camX - 1);
            moved = true;
        }
        if (keys[SDL_SCANCODE_D])
        {
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

    void Game::RenderMainMenu()
    {

        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        int menu_w = w / 4;
        int menu_h = static_cast<int>(mMainMenu.GetOptionCount()) * 50;
        int menu_x = (w - menu_w) / 2;
        int menu_y = (h - menu_h) / 2;

        int title_x = (w - mTitleText.GetWidth()) / 2;
        mTitleText.Draw(title_x, menu_y - 80);

        SDL_Renderer *renderer = mGameView->GetRenderer();
        mMainMenu.DrawMenu(renderer, menu_x, menu_y, menu_w, menu_h);
    }

    // Z-layer ordering. Put here for future reference of probable Game draw logic
    void Game::RenderOverworld()
    {
        // Layer 0 — Tiles (bottom)
        RenderWorld(*mOverworldGrid, mCamX, mCamY);

        // Layer 1 — all agents including player
        int tw = static_cast<int>(mOverworldGrid->GetTileWidth());
        int th = static_cast<int>(mOverworldGrid->GetTileHeight());

        for (size_t i = 0; i < mOverWorld->GetNumAgents(); ++i)
        {
            const AgentBase &agent = mOverWorld->GetAgent(i);
            const WorldPosition &pos = agent.GetLocation().AsWorldPosition();

            int screen_x = (static_cast<int>(pos.CellX()) - mCamX) * tw;
            int screen_y = (static_cast<int>(pos.CellY()) - mCamY) * th;

            // Pick sprite based on whether this is the player or an NPC
            const std::string &sprite = (&agent == mOverworldPlayer) ? "player" : mOverWorld->GetAgentSpriteName();
            mImageManager->DrawImage(sprite, screen_x, screen_y, tw, th);
        }

        // Layer 2 — UI/HUD
        RenderHotbar(mOverworldPlayer->GetInventory());
        if (mShowBackpack) RenderBackpack(mOverworldPlayer->GetInventory());
    }

    void Game::RenderDungeon()
    {
        RenderWorld(*mDungeonGrid, mDungeonCamX, mDungeonCamY);

        int tw = static_cast<int>(mDungeonGrid->GetTileWidth());
        int th = static_cast<int>(mDungeonGrid->GetTileHeight());

        for (size_t i = 0; i < mDungeonWorld->GetNumAgents(); ++i)
        {
            const AgentBase &agent = mDungeonWorld->GetAgent(i);
            const WorldPosition &pos = agent.GetLocation().AsWorldPosition();

            int screen_x = (static_cast<int>(pos.CellX()) - mDungeonCamX) * tw;
            int screen_y = (static_cast<int>(pos.CellY()) - mDungeonCamY) * th;

            const std::string &sprite = (&agent == mDungeonPlayer) ? "player" : "dun_monster";
            mImageManager->DrawImage(sprite, screen_x, screen_y, tw, th);
        }

        RenderHotbar(mDungeonPlayer->GetInventory());
        RenderPickupMessage();
        if (mShowBackpack) RenderBackpack(mDungeonPlayer->GetInventory());
    }

    void Game::RenderWorld(const ImageGrid &grid, int camX, int camY)
    {
        grid.DrawViewport(*mImageManager, camX, camY, mGameView->GetWidth(), mGameView->GetHeight());
    }

    void Game::RenderPaused()
    {
        SDL_Renderer *renderer = mGameView->GetRenderer();
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

    void Game::RenderSettings()
    {
        // TODO: render settings screen
    }

    void Game::RenderHotbar(const Inventory &inventory) {
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        // Draw the inventory bar image centered at the bottom
        int bar_w = 640;
        int bar_h = 64;
        int bar_x = (w - bar_w) / 2;
        int bar_y = h - bar_h - 10; // 10px padding from bottom

        mImageManager->DrawImage("inventory_bar", bar_x, bar_y, bar_w, bar_h);

        // Draw items in each hotbar slot
        const auto& slots = inventory.GetInventoryArray();
        int slot_size = bar_w / static_cast<int>(Inventory::HOTBAR_SIZE);

        for (size_t i = 0; i < Inventory::HOTBAR_SIZE; ++i) {
            const auto& slot = slots[i];
            if (!slot.IsEmpty()) {
                const Item* item = slot.GetItem();
                int item_x = bar_x + static_cast<int>(i) * slot_size + (slot_size - 48) / 2;
                int item_y = bar_y + (bar_h - 48) / 2;

                // Draw item icon if loaded — uses the item's image path as key
                mImageManager->DrawImage(item->GetName(), item_x, item_y, 48, 48);
                //std::cout << item->GetName() << std::endl;
            }
        }

        // Highlight the selected hotbar slot
        SDL_Renderer* renderer = mGameView->GetRenderer();
        size_t selected =  inventory.GetHandSlotIndex();
        int sel_x = bar_x + static_cast<int>(selected) * slot_size;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 180);
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_Rect highlight = {sel_x, bar_y, slot_size, bar_h};
        SDL_RenderDrawRect(renderer, &highlight);
    }


    void Game::RenderBackpack(const Inventory& inventory) {
        SDL_Renderer* renderer = mGameView->GetRenderer();
        int w = mGameView->GetWidth();
        int h = mGameView->GetHeight();

        // Semi-transparent dark overlay
        SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
        SDL_SetRenderDrawColor(renderer, 0, 0, 0, 180);
        SDL_Rect overlay = {0, 0, w, h};
        SDL_RenderFillRect(renderer, &overlay);

        const auto& slots = inventory.GetInventoryArray();

        int slot_size = 64;
        int padding = 4;
        int cols = static_cast<int>(Inventory::ITEMS_PER_ROW);
        int rows = static_cast<int>(Inventory::BACKPACK_SIZE) / cols;

        int grid_w = cols * (slot_size + padding) - padding;
        int grid_h = rows * (slot_size + padding) - padding;
        int grid_x = (w - grid_w) / 2;
        int grid_y = (h - grid_h) / 2;

        for (int row = 0; row < rows; ++row) {
            for (int col = 0; col < cols; ++col) {
                size_t index = Inventory::HOTBAR_SIZE + row * cols + col;

                int x = grid_x + col * (slot_size + padding);
                int y = grid_y + row * (slot_size + padding);

                // Slot background
                SDL_SetRenderDrawColor(renderer, 40, 40, 50, 220);
                SDL_Rect slot_bg = {x, y, slot_size, slot_size};
                SDL_RenderFillRect(renderer, &slot_bg);

                // Slot border
                SDL_SetRenderDrawColor(renderer, 100, 100, 120, 255);
                SDL_RenderDrawRect(renderer, &slot_bg);

                // Draw item if present
                if (!slots[index].IsEmpty()) {
                    const Item* item = slots[index].GetItem();
                    int item_x = x + (slot_size - 48) / 2;
                    int item_y = y + (slot_size - 48) / 2;
                    mImageManager->DrawImage(item->GetName(), item_x, item_y, 48, 48);
                }

                // Cursor highlight
                if (row == mBackpackCursorRow && col == mBackpackCursorCol) {
                    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 220);
                    SDL_Rect cursor = {x - 2, y - 2, slot_size + 4, slot_size + 4};
                    SDL_RenderDrawRect(renderer, &cursor);
                    SDL_Rect cursor_inner = {x - 1, y - 1, slot_size + 2, slot_size + 2};
                    SDL_RenderDrawRect(renderer, &cursor_inner);
                }
            }
        }
    }

    void Game::RenderPickupMessage() {
        if (mPickupMessage.empty()) return;

        Uint32 elapsed = SDL_GetTicks() - mPickupMessageTime;
        if (elapsed > 1000) {
            mPickupMessage.clear();
            return;
        }

        int w = mGameView->GetWidth();

        mPickupText.SetContent(mPickupMessage);
        int text_x = (w - mPickupText.GetWidth()) / 2;
        mPickupText.Draw(text_x, 20);
    }


    size_t Game::KeyToAction(SDL_Keycode key) {
        switch (key) {
        case SDLK_w: return 1; // MOVE_UP
        case SDLK_s: return 2; // MOVE_DOWN
        case SDLK_a: return 3; // MOVE_LEFT
        case SDLK_d: return 4; // MOVE_RIGHT
        default:     return 0; // REMAIN_STILL
        }
    }

    void Game::ProcessPlayerMove(SDL_Keycode key) {
        size_t action = KeyToAction(key);
        if (action == 0) return;

        if (mState == GameState::OVERWORLD) {
            mOverWorld->DoAction(*mOverworldPlayer, action);

            WorldPosition pos = mOverworldPlayer->GetLocation().AsWorldPosition();
            mPlayerX = static_cast<int>(pos.CellX());
            mPlayerY = static_cast<int>(pos.CellY());

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
            // Snapshot inventory count before move
            size_t items_before = 0;
            const auto& slots = mDungeonPlayer->GetInventory().GetInventoryArray();
            for (const auto& slot : slots) {
                if (!slot.IsEmpty()) items_before += slot.GetQuantity();
            }

            mDungeonWorld->DoAction(*mDungeonPlayer, action);

            // Check if inventory changed
            size_t items_after = 0;
            for (const auto& slot : slots) {
                if (!slot.IsEmpty()) items_after += slot.GetQuantity();
            }
            if (items_after > items_before) {
                // Find the newest item — scan for an item that wasn't there before
                // Simplest: just grab the hand or last non-empty slot
                for (const auto& slot : slots) {
                    if (!slot.IsEmpty()) {
                        mPickupMessage = "Picked up: " + slot.GetItem()->GetName();
                    }
                }
                mPickupMessageTime = SDL_GetTicks();
            }

            WorldPosition pos = mDungeonPlayer->GetLocation().AsWorldPosition();
            mDungeonPlayerX = static_cast<int>(pos.CellX());
            mDungeonPlayerY = static_cast<int>(pos.CellY());

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