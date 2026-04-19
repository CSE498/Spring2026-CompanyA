/// @file WebInterface.hpp
/// @brief Header for the main UI shell of the Group 18 WebUI demo application
/// @details This file was modified with AI assistance for
/// inventory system enhancements, menu management, and UI state transitions.


#pragma once

#include "../../../core/WorldBase.hpp"
#include "../InputManager.hpp"
#include "../internal/IDomElement.hpp"

#include <emscripten/emscripten.h>
#include <emscripten/val.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cse498 {

class WebLayout;
class WebTextbox;
class WebCanvas;
class WebButton;
class WebImage;
class InteractiveWorld;
class DungeonWorld;
class Item;
class Inventory;

/// @class WebInterface
/// @brief Web-based application shell for the WebUI demo.
/// @details This class owns an InteractiveWorld instance and a DungeonWorld, manages WebUI state, and renders the game.
class WebInterface {

public:
    /// @brief Constructs a WebInterface application shell.
    /// @param overworld Unique pointer to the owned InteractiveWorld instance.
    /// @param dungeonWorld Unique pointer to the owned dungeon world instance.
    explicit WebInterface(std::unique_ptr<InteractiveWorld> overworld, std::unique_ptr<DungeonWorld> dungeonWorld);
    ~WebInterface() = default;

    /// @brief Selects an action based on user input.
    /// @param grid The world grid.
    /// @return The selected action ID.
    [[nodiscard]] const char SelectAction(const WorldGrid& grid);

    /// @brief Runs one frame of the application.
    /// @param currentTimeMs Current time stamp in milliseconds.
    void RunFrame(double currentTimeMs);

    /// @brief Test whether the interface is currently paused.
    [[nodiscard]] bool IsPaused() const;

    /// @brief Access the owned world instance.
    /// @return Reference to the underlying world.
    [[nodiscard]] WorldBase& GetWorld() const;

    /// @brief Gets the last action executed during the most recent frame.
    [[nodiscard]] const char GetLastActionChar() const;

    /// @brief Renders the current frame.
    void RenderFrame();

    /// @brief Handles pause and resume transitions.
    void HandlePause();

    /// @brief Sets the hotbar index for the current player's inventory.
    /// @param slotIndex The hotbar slot index (0-9).
    void SetPlayerHotbarIndex(size_t slotIndex);

    /// @brief Opens the inventory menu as a paused state.
    void OpenInventory();

    /// @brief Handles inventory slot click events and performs item swaps.
    /// @param slotIndex The inventory slot that was clicked.
    void OnInventorySlotClick(size_t slotIndex);

    /// @brief Current WebUI state (exposed for testing).
    enum class WebState { MAIN_MENU, OVERWORLD, DUNGEON, PAUSED, SETTINGS, INVENTORY, QUIT };

    /// @brief Get the current player from the active world.
    /// @return Pointer to the current player, or nullptr if no world is active.
    [[nodiscard]] PlayerAgent* GetCurrentPlayer() const;

    /// @brief Get the current WebUI state.
    [[nodiscard]] WebState GetCurrentState() const { return mState; }

private:
    /// @brief Current game state for the WebUI state machine.
    WebState mState = WebState::MAIN_MENU;

    /// @brief Last active state before pause or settings.
    WebState mPreviousState = WebState::MAIN_MENU;

    /// @brief The actual gameplay state (OVERWORLD or DUNGEON) to return to when resuming.
    WebState mGameState = WebState::OVERWORLD;

    /// @brief Input manager for handling user input.
    InputManager mInputManager{*this};

    /// @brief Vector of DOM elements managed by the interface.
    std::vector<std::unique_ptr<IDomElement>> mElements{};

    /// @brief Current points score.
    int mPoints{0};

    /// @brief Set when a UI quit button was pressed to request a quit action.
    bool mQuitRequested = false;

    /// @brief The last action selected and executed during a frame.
    char mLastActionChar{0};

    /// @brief Timer to control player action rate
    double mPlayerTimer = 0.0;

    /// @brief Timer to control agent action rate
    double mAgentTimer = 0.0;

    /// @brief Owned interactive world instance used by this app.
    std::unique_ptr<InteractiveWorld> mInteractiveWorld;

    /// @brief Owned dungeon world instance used by this app (kept for compatibility).
    std::unique_ptr<DungeonWorld> mDungeon;

    /// @brief Timestamp of the last rendered frame in milliseconds.
    double mLastTimeMs{0.0};

    enum ActionId : char {
        ACTION_NONE = '\0',
        ACTION_UP = 'w',
        ACTION_DOWN = 's',
        ACTION_LEFT = 'a',
        ACTION_RIGHT = 'd',
        ACTION_INTERACT = 'e',
        ACTION_QUIT = 'q',
    };

    /// @brief Root layout element.
    WebLayout* mRoot = nullptr;

    /// @brief Main menu layout.
    WebLayout* mMainMenu = nullptr;

    /// @brief Pause menu layout.
    WebLayout* mPauseMenu = nullptr;

    /// @brief Settings menu layout.
    WebLayout* mSettingsMenu = nullptr;

    /// @brief Inventory menu layout.
    WebLayout* mInventoryMenu = nullptr;

    /// @brief Canvas for rendering the game grid.
    WebCanvas* mCanvas = nullptr;

    /// @brief Textbox displaying points.
    WebTextbox* mHUDTextbox = nullptr;

    /// @brief Map of symbols to file paths for the overworld.
    std::unordered_map<char, std::string> mSymbolPathOverworld{};

    /// @brief Map of symbols to file paths for the dungeon.
    std::unordered_map<char, std::string> mSymbolPathDungeon{};

    /// @brief Global map of file paths to loaded bitmaps.
    std::unordered_map<std::string, emscripten::val> mPathBitmaps{};

    /// @brief Struct to hold inventory slot image data.
    struct InventorySlotImage {
        WebImage* image; ///< Pointer to the WebImage element
        size_t slotIndex; ///< Inventory slot index (relative to inventory, starts at 10 for backpack)
    };

    /// @brief Vector of inventory slot images for dynamic updates and interaction.
    std::vector<InventorySlotImage> mInventoryImages{};


    /// @brief Sets visible UI modes using the current state.
    void UpdateLayoutVisibility();

    /// @brief Initialize main menu layout and button callbacks.
    void SetupMainMenu();

    /// @brief Initialize pause menu layout and button callbacks.
    void SetupPauseMenu();

    /// @brief Initialize settings menu layout and button callbacks.
    void SetupSettingsMenu();

    /// @brief Initialize inventory menu layout and item display.
    void SetupInventoryMenu();

    /// @brief Transition to a new WebUI state.
    /// @param newState Target state.
    void TransitionTo(WebState newState);

    /// @brief Enter paused state from the current active state.
    void Pause();

    /// @brief Return from pause or settings to the previous state.
    void Resume();

    /// @brief Render the overworld canvas and HUD.
    void RenderOverworld();

    /// @brief Render the dungeon canvas and HUD.
    void RenderDungeon();

    /// @brief Render HUD elements like player inventory.
    void RenderHUD();

    /// @brief Draws the world grid on the canvas.
    /// @param grid The world grid to draw.
    /// @param itemIds IDs of items to display.
    /// @param agentIds IDs of agents to display.
    void DrawGrid(const WorldGrid& grid, const std::vector<size_t>& itemIds, const std::vector<size_t>& agentIds);

    /// @brief Gets the current active world based on the current state.
    /// @return Pointer to the current world, or nullptr if no world is active.
    [[nodiscard]] WorldBase* GetCurrentWorld() const;

    /// @brief Gets the symbol-to-path map for the current world.
    /// @return Reference to the symbol path map for the active world.
    [[nodiscard]] std::unordered_map<char, std::string>& GetSymbolPathMap();

    /// @brief Gets the file path for a symbol in the current world.
    /// @param symbol The symbol character.
    /// @return The file path for the symbol, or empty string if not found.
    [[nodiscard]] std::string GetImagePath(char symbol);

    /// @brief Gets or loads a bitmap for the given file path.
    /// @param path The file path to load.
    /// @return The loaded or cached bitmap value.
    [[nodiscard]] emscripten::val GetOrLoadBitmap(const std::string& path);

    /// @brief Populates the inventory menu with current player inventory items.
    void PopulateInventoryMenu();
};

} // namespace cse498
