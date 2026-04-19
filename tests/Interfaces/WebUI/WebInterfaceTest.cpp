#ifdef __EMSCRIPTEN__

#include "../../../source/Interfaces/WebUI/interface/WebInterface.hpp"
#include "../../../source/Worlds/Dungeon/DungeonWorld.hpp"
#include "../../../source/Worlds/Hub/InteractiveWorld.hpp"
#include "../../../third-party/Catch/single_include/catch2/catch.hpp"

#include <iostream>

using namespace cse498;

/// @brief Helper to create a keyboard event
/// @param key The key to simulate
/// @param type The type of keyboard event e.g. keydown or keyup
EM_JS(void, keyboardEvent, (const char* key, const char* type), {
    const keyStr = UTF8ToString(key);
    const typeStr = UTF8ToString(type);
    const event = new KeyboardEvent(typeStr, {
        key : keyStr,
        code : keyStr,
    });
    window.dispatchEvent(event);
});

/// @brief Helper to click a button by its text content
/// @param buttonText The text content of the button to click
EM_JS(void, clickButton, (const char* buttonText), {
    const textStr = UTF8ToString(buttonText);
    const buttons = document.querySelectorAll('button');
    // clang-format off
    for (let btn of buttons) {
        if (btn.textContent === textStr) {
            btn.click();
            break;
        }
    }
    // clang-format on
});

class TestingApp : public WebInterface {
public:
    TestingApp() : WebInterface(std::make_unique<InteractiveWorld>(), std::make_unique<DungeonWorld>()) {}

    bool IsInMainMenu() const { return GetCurrentState() == WebState::MAIN_MENU; }
    bool IsInOverworld() const { return GetCurrentState() == WebState::OVERWORLD; }
    bool IsInDungeon() const { return GetCurrentState() == WebState::DUNGEON; }
    bool IsInPauseMenu() const { return GetCurrentState() == WebState::PAUSED; }
    bool IsInSettingsMenu() const { return GetCurrentState() == WebState::SETTINGS; }
    bool IsInInventoryMenu() const { return GetCurrentState() == WebState::INVENTORY; }
    bool IsQuit() const { return GetCurrentState() == WebState::QUIT; }

    /// @brief Get player position for movement tests
    [[nodiscard]] WorldPosition GetPlayerPosition() const {
        PlayerAgent* player = GetCurrentPlayer();
        if (player) {
            return player->GetLocation().AsWorldPosition();
        }
        return WorldPosition{0, 0};
    }
};

TEST_CASE("WebInterface Initial State") {
    TestingApp app{};

    CHECK(app.IsInMainMenu());
    CHECK(app.IsPaused());
}

TEST_CASE("WebInterface Main Menu to Overworld") {
    TestingApp app{};

    CHECK(app.IsInMainMenu());

    // Click "New Game" button to transition to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    // Should now be in overworld
    CHECK(app.IsInOverworld());
    CHECK(!app.IsPaused());
}

TEST_CASE("WebInterface Pause and Resume from Overworld") {
    TestingApp app{};

    // Start in main menu
    CHECK(app.IsInMainMenu());

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    CHECK(app.IsInOverworld());
    CHECK(!app.IsPaused());

    // Pause with Escape
    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    CHECK(app.IsInPauseMenu());
    CHECK(app.IsPaused());

    // Resume by clicking "Resume" button
    clickButton("Resume");
    app.RunFrame(48.0);

    CHECK(app.IsInOverworld());
    CHECK(!app.IsPaused());
}

TEST_CASE("WebInterface Open Settings from Pause Menu") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    // Pause
    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    CHECK(app.IsInPauseMenu());

    // Click Settings button
    clickButton("Settings");
    app.RunFrame(48.0);

    CHECK(app.IsInSettingsMenu());
    CHECK(app.IsPaused());
}

TEST_CASE("WebInterface Close Settings") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    // Pause and go to settings
    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    clickButton("Settings");
    app.RunFrame(48.0);

    CHECK(app.IsInSettingsMenu());

    // Click "Back" button to return to pause menu
    clickButton("Back");
    app.RunFrame(64.0);

    CHECK(app.IsInPauseMenu());
}

TEST_CASE("WebInterface Quit from Main Menu") {
    TestingApp app{};

    CHECK(app.IsInMainMenu());

    // Click Quit button
    clickButton("Quit");
    app.RunFrame(16.0);

    CHECK(app.IsQuit());
}

TEST_CASE("WebInterface Inventory Menu - Cannot Open in Overworld") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    CHECK(app.IsInOverworld());

    // Try to press 'i' to open inventory - should not work in overworld
    keyboardEvent("i", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("i", "keyup");

    // Should still be in overworld, not inventory
    CHECK(app.IsInOverworld());
    CHECK(!app.IsInInventoryMenu());
}

TEST_CASE("WebInterface Open Inventory Menu in Dungeon") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    CHECK(app.IsInOverworld());

    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    // Enter dungeon
    clickButton("Go to Dungeon");
    app.RunFrame(32.0);

    CHECK(app.IsInDungeon());

    // Press 'i' to open inventory in dungeon
    keyboardEvent("i", "keydown");
    app.RunFrame(48.0);
    keyboardEvent("i", "keyup");

    CHECK(app.IsInInventoryMenu());
    CHECK(app.IsPaused());
}

TEST_CASE("WebInterface Close Inventory Menu in Dungeon") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    // Enter dungeon
    clickButton("Go to Dungeon");
    app.RunFrame(32.0);

    // Open inventory
    keyboardEvent("i", "keydown");
    app.RunFrame(48.0);
    keyboardEvent("i", "keyup");

    CHECK(app.IsInInventoryMenu());

    // Click Close button to return to dungeon
    clickButton("Close");
    app.RunFrame(64.0);

    CHECK(app.IsInDungeon());
    CHECK(!app.IsPaused());
}

TEST_CASE("WebInterface toggle Inventory Menu in Dungeon") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    // Enter dungeon
    clickButton("Go to Dungeon");
    app.RunFrame(32.0);

    CHECK(app.IsInDungeon());

    // Open inventory
    keyboardEvent("i", "keydown");
    app.RunFrame(48.0);
    keyboardEvent("i", "keyup");

    CHECK(app.IsInInventoryMenu());

    // Press 'i' again to toggle back
    keyboardEvent("i", "keydown");
    app.RunFrame(64.0);
    keyboardEvent("i", "keyup");

    CHECK(app.IsInDungeon());
    CHECK(!app.IsPaused());
}

TEST_CASE("WebInterface Player Movement - Basic Directions") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    const WorldPosition startPos = app.GetPlayerPosition();

    // Test moving down
    keyboardEvent("s", "keydown");
    app.RunFrame(300.0); // Exceed action interval
    keyboardEvent("s", "keyup");

    CHECK(app.GetPlayerPosition() == startPos.Down());

    // Test moving up
    keyboardEvent("w", "keydown");
    app.RunFrame(600.0); // Exceed action interval
    keyboardEvent("w", "keyup");

    CHECK(app.GetPlayerPosition() == startPos);

    // Test moving right
    keyboardEvent("d", "keydown");
    app.RunFrame(900.0); // Exceed action interval
    keyboardEvent("d", "keyup");

    CHECK(app.GetPlayerPosition() == startPos.Right());

    // Test moving left
    keyboardEvent("a", "keydown");
    app.RunFrame(1200.0); // Exceed action interval
    keyboardEvent("a", "keyup");

    CHECK(app.GetPlayerPosition() == startPos);
}

TEST_CASE("WebInterface Hotbar Selection with Hand Slot Verification") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    // Enter dungeon
    clickButton("Go to Dungeon");
    app.RunFrame(32.0);

    PlayerAgent* player = app.GetCurrentPlayer();
    REQUIRE(player != nullptr);

    // Test number keys for hotbar selection (1-9)
    // '1' should map to slot 0
    keyboardEvent("1", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("1", "keyup");

    CHECK(player->GetInventory().GetHandSlotIndex() == 0);

    // '5' should map to slot 4
    keyboardEvent("5", "keydown");
    app.RunFrame(48.0);
    keyboardEvent("5", "keyup");

    CHECK(player->GetInventory().GetHandSlotIndex() == 4);

    // '9' should map to slot 8
    keyboardEvent("9", "keydown");
    app.RunFrame(64.0);
    keyboardEvent("9", "keyup");

    CHECK(player->GetInventory().GetHandSlotIndex() == 8);

    // '0' should map to slot 9
    keyboardEvent("0", "keydown");
    app.RunFrame(80.0);
    keyboardEvent("0", "keyup");

    CHECK(player->GetInventory().GetHandSlotIndex() == 9);
}

TEST_CASE("WebInterface Movement Paused in Menu") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    const WorldPosition startPos = app.GetPlayerPosition();

    // Pause the game
    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    CHECK(app.IsInPauseMenu());

    // Try to move - should not move while paused
    keyboardEvent("s", "keydown");
    app.RunFrame(300.0);
    keyboardEvent("s", "keyup");

    CHECK(app.GetPlayerPosition() == startPos);
}

TEST_CASE("WebInterface Enter and Exit Dungeon") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    CHECK(app.IsInOverworld());

    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    // Enter dungeon by clicking dungeon entrance (simulated by "Enter" button)
    clickButton("Go to Dungeon");
    app.RunFrame(32.0);

    CHECK(app.IsInDungeon());

    keyboardEvent("Escape", "keydown");
    app.RunFrame(32.0);
    keyboardEvent("Escape", "keyup");

    // Exit dungeon back to overworld
    clickButton("Go to Overworld");
    app.RunFrame(48.0);

    CHECK(app.IsInOverworld());
}

TEST_CASE("WebInterface Invalid Key Handling") {
    TestingApp app{};

    // Go to overworld
    clickButton("New Game");
    app.RunFrame(16.0);

    const WorldPosition startPos = app.GetPlayerPosition();

    std::vector<std::string> invalidKeys = {"p", "Enter", "Shift", "Tab"};

    double currentTime = 32.0;
    for (const std::string& key: invalidKeys) {
        keyboardEvent(key.c_str(), "keydown");
        app.RunFrame(currentTime);
        keyboardEvent(key.c_str(), "keyup");

        // Position should not change
        CHECK(app.GetPlayerPosition() == startPos);
        CHECK(app.IsInOverworld());
        currentTime += 300.0; // Increment time for next input
    }
}

#endif
