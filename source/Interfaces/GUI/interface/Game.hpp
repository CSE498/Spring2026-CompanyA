/**
 * @file Game.hpp
 * @brief Top-level Game class that owns and manages core game systems.
 *
 * This file is part of the Spring 2026, CSE 498, section 2 course project.
 *
 * @note Status: PROPOSAL
 * @note Made via reference to Claude Sonnet 4.6
 */

#pragma once

#include <memory>
#include <string>

#include "../../../Analyze/AnalyticsManager.hpp"
#include "../../../Analyze/StatsTracker.hpp"
#include "../GameView.hpp"
#include "../ImageGrid.hpp"
#include "../ImageManager.hpp"
#include "../Menu.hpp"
#include "../Text.hpp"
// Group 17 build-fix: these two includes were relative paths that did not resolve
// from this file's location on upstream @c group17_inv_and_dungeon. They now use
// source-root-relative paths (the @c source/ directory is an include root in
// root-level @c CMakeLists.txt) so this TU actually compiles.
#include "Worlds/Dungeon/DungeonWorld.hpp"
#include "OverWorld.hpp"
#include "Agents/Classic/PlayerAgent.hpp"

namespace cse498
{

    /**
     * @enum GameState
     * @brief Represents the current state of the game.
     */
    enum class GameState
    {
        MAIN_MENU, /// Main menu screen
        OVERWORLD, /// Flat interactive world with dungeon entrance
        DUNGEON,   /// Procedurally generated dungeon world
        PAUSED,    /// Paused state (reachable from OVERWORLD or DUNGEON)
        SETTINGS,  /// Settings screen (placeholder)
        STATS,     /// Contains information captured in gameplay
        QUIT       /// Exit state
    };

    /**
     * @class Game
     * @brief Core class that manages game state, rendering, input, and world systems.
     *
     * The Game class is responsible for:
     * - Initializing the rendering system and UI
     * - Managing game state transitions
     * - Updating world logic and agents
     * - Rendering different game states (menu, overworld, dungeon, etc.)
     */
    class Game
    {
    private:
        std::shared_ptr<GameView> mGameView; /// Main rendering and window system

        GameState mState = GameState::MAIN_MENU; /// Current game state
        GameState mPreviousState = GameState::MAIN_MENU; /// Used to resume after pause

        // -------------------------
        // Constants
        // -------------------------
        static constexpr int kDefaultWindowWidth = 800;
        static constexpr int kDefaultWindowHeight = 600;

        static constexpr int kInitialPlayerX = 1;
        static constexpr int kInitialPlayerY = 1;

        // -------------------------
        // Main menu UI
        // -------------------------
        Menu mMainMenu; /// Main menu options
        Text mTitleText; /// Title text displayed on main menu

        // -------------------------
        // Pause menu UI
        // -------------------------
        Menu mPauseMenu; /// Pause menu options
        Text mPauseText; /// Pause screen title text

        // -------------------------
        // Backpack/Inventory
        // -------------------------
        int mBackpackCursorRow = 0;    /// Current backpack cursor row
        int mBackpackCursorCol = 0;    /// Current backpack cursor column
        std::string mPickupMessage;    /// Message on pickup
        Text mPickupText;              /// Pickup notification text
        Uint32 mPickupMessageTime = 0; /// When the message was set

        // -------------------------
        // Overworld state
        // -------------------------
        std::unique_ptr<ImageManager> mImageManager; /// Handles image loading and rendering
        std::unique_ptr<ImageGrid> mOverworldGrid; /// Renderable grid for overworld tiles
        std::unique_ptr<OverWorld> mOverWorld; /// Overworld game logic
        std::unique_ptr<DungeonWorld> mDungeonWorld; /// Dungeon world game logic

        int mCamX = 0; /// Camera X position in tile coordinates
        int mCamY = 0; /// Camera Y position in tile coordinates

        int mPlayerX = kInitialPlayerX; /// Player X position in overworld tile coordinates
        int mPlayerY = kInitialPlayerY; /// Player Y position in overworld tile coordinates

        // Player agent pointers — worlds own the agents, Game holds raw pointers for access
        PlayerAgent* mOverworldPlayer = nullptr;
        PlayerAgent* mDungeonPlayer = nullptr;

        // -------------------------
        // Dungeon state
        // -------------------------
        std::unique_ptr<ImageGrid> mDungeonGrid; /// Renderable grid for dungeon tiles

        int mDungeonCamX = 0; /// Dungeon camera X position in tile coordinates
        int mDungeonCamY = 0; /// Dungeon camera Y position in tile coordinates

        int mDungeonPlayerX = kInitialPlayerX; /// Player X position in dungeon tile coordinates
        int mDungeonPlayerY = kInitialPlayerY; /// Player Y position in dungeon tile coordinates

        // -------------------------
        // Stats state
        // -------------------------
        std::shared_ptr<AnalyticsManager> mAnalyticsManager; /// Manages gameplay stats and logs
        std::unique_ptr<StatsTracker> mStatsTracker; /// Used to build GUI-friendly summaries from analytics data
        DashboardSnapshot mDashboardSnapshot; /// Stats snapshot for rendering
        Text mStatsText; /// Text object used for stats screen

        // -------------------------
        // Runtime flags
        // -------------------------
        bool mRunning = false; /// Controls main game loop execution
        bool mTurnTaken = false; /// True when player acts; consumed by UpdateOverworld
        bool mShowBackpack = false; /// Toggle backpack overlay

        // -------------------------
        // Core loop methods
        // -------------------------

        /**
         * @brief Handle all SDL input events.
         */
        void HandleEvents();

        /**
         * @brief Update logic for each game state.
         */
        void UpdateMainMenu();
        void UpdateOverworld();
        void UpdateDungeon();
        void UpdatePaused();
        void UpdateSettings();
        void UpdateStats();

        /**
         * @brief Render functions for each game state.
         */
        void RenderMainMenu();
        void RenderOverworld();
        void RenderDungeon();
        void RenderPaused();
        void RenderSettings();
        void RenderStats();
        void RenderHotbar(const Inventory& inventory);
        void RenderBackpack(const Inventory& inventory);
        void RenderPickupMessage();

        /**
         * @brief Convert SDL keycode to world action ID.
         * @param key SDL keycode (SDLK_w, SDLK_a, SDLK_s, SDLK_d)
         * @return Action ID matching WorldBase action conventions, 0 = remain still
         */
        size_t KeyToAction(SDL_Keycode key);

        /**
         * @brief Process player movement input.
         * @param key SDL keycode representing movement input
         */
        void ProcessPlayerMove(SDL_Keycode key);

        /**
         * @brief Update camera position for a given grid.
         * @param grid Reference to the grid
         * @param camX Camera X position (modified)
         * @param camY Camera Y position (modified)
         */
        [[deprecated]] void UpdateWorld(ImageGrid &grid, int &camX, int &camY);

        /**
         * @brief Render a world grid within the current viewport.
         * @param grid Reference to the grid (read-only)
         * @param camX Camera X position
         * @param camY Camera Y position
         */
        void RenderWorld(const ImageGrid &grid, int camX, int camY);

        // -------------------------
        // Setup and state helpers
        // -------------------------

        /**
         * @brief Initialize main menu options.
         */
        void SetupMainMenu();

        /**
         * @brief Initialize pause menu options.
         */
        void SetupPauseMenu();

        /**
         * @brief Initialize overworld systems and grid.
         */
        void SetupOverworld();

        /**
         * @brief Initialize dungeon systems and grid.
         */
        void SetupDungeon();

        /**
         * @brief Rebuild the dungeon ImageGrid after a level change.
         */
        void RebuildDungeonGrid();

    /**
     * @brief Transition to a new game state.
     * @param new_state Target state
     */
    void TransitionTo(GameState new_state);

    /**
     * @brief Enter paused state.
     */
    void Pause();

    /**
     * @brief Resume previous state from pause.
     */
    void Resume();

    public:
        /**
         * @brief Construct a new Game instance.
         * @param title Window title
         * @param width Window width in pixels
         * @param height Window height in pixels
         */
        Game(const std::string &title = "Slay the Dungeon", int width = kDefaultWindowWidth,
             int height = kDefaultWindowHeight) :
            mGameView(std::make_shared<GameView>(title, width, height)), mTitleText(nullptr), mPauseText(nullptr),
            mPickupText(nullptr), mStatsText(nullptr)
        {
        }

    /**
     * @brief Default destructor.
     */
    ~Game() = default;

    Game(const Game&) = delete;
    Game& operator=(const Game&) = delete;

    Game(Game&&) = default;
    Game& operator=(Game&&) = default;

    /**
     * @brief Initialize SDL, window, renderer, and menus.
     * @return true if initialization succeeds, false otherwise
     */
    bool Initialize();

    /**
     * @brief Run the main game loop.
     */
    void Run();

    /**
     * @brief Signal the game loop to stop.
     */
    void Quit() { mRunning = false; }

    // -------------------------
    // Accessors
    // -------------------------

    /**
     * @brief Get the GameView instance.
     * @return Shared pointer to GameView
     */
    [[nodiscard]] std::shared_ptr<GameView> GetGameView() const { return mGameView; }

        /**
         * @brief Get the current game state.
         * @return Current GameState
         */
        [[nodiscard]] GameState GetState() const { return mState; }
    };

} // namespace cse498