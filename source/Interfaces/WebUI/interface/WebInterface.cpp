/// @file WebInterface.cpp
/// @brief Main UI shell for the Group 18 WebUI demo application
/// @details This file was modified with AI assistance for
/// inventory system enhancements, menu management, and UI state transitions.

#ifdef __EMSCRIPTEN__

#include "./WebInterface.hpp"
#include "../../../../third-party/gsl/gsl"
#include "../../../Agents/Classic/PlayerFeatures/Inventory.hpp"
#include "../../../Agents/PacingAgent.hpp"
#include "../../../Worlds/Dungeon/DungeonWorld.hpp"
#include "../../../Worlds/Hub/InteractiveWorld.hpp"
#include "../../../Worlds/Hub/ItemType.hpp"
#include "../../../core/WorldBase.hpp"
#include "../../../core/item/Item.hpp"
#include "../../../tools/Color.hpp"
#include "../WebButton/WebButton.hpp"
#include "../WebCanvas/WebCanvas.hpp"
#include "../WebImage/WebImage.hpp"
#include "../WebLayout/WebLayout.hpp"
#include "../WebTextbox/WebTextbox.hpp"
#include "../WebUtils.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <sstream>

using namespace cse498;

namespace {
// Color constants
constexpr cse498::Color kDimGray = cse498::Color::FromRGB255(0xcc, 0xcc, 0xcc);
constexpr cse498::Color kCanvasBg = cse498::Color::FromRGB255(0x0c, 0x10, 0x17);
constexpr cse498::Color kButtonColorDark = cse498::Color::FromRGB255(0x33, 0x33, 0x33);
constexpr cse498::Color kButtonColorMedium = cse498::Color::FromRGB255(0x44, 0x44, 0x44);
constexpr cse498::Color kButtonTextColorWhite = cse498::Color::FromRGB255(0xff, 0xff, 0xff);
static_assert(kDimGray.R() == 0xcc && kDimGray.G() == 0xcc && kDimGray.B() == 0xcc);
static_assert(kCanvasBg.R() == 0x0c && kCanvasBg.G() == 0x10 && kCanvasBg.B() == 0x17);
static_assert(kButtonColorDark.R() == 0x33 && kButtonColorDark.G() == 0x33 && kButtonColorDark.B() == 0x33);
static_assert(kButtonColorMedium.R() == 0x44 && kButtonColorMedium.G() == 0x44 && kButtonColorMedium.B() == 0x44);
static_assert(kButtonTextColorWhite.R() == 0xff && kButtonTextColorWhite.G() == 0xff &&
              kButtonTextColorWhite.B() == 0xff);

// HUD and UI positioning constants
constexpr float kHUDPositionX = 10.0f;
constexpr float kHUDPositionY = 50.0f;
constexpr int kHUDFontSize = 24;

// Menu layout constants
constexpr int kMenuMainSpacing = 15;
constexpr int kMenuPauseSpacing = 10;
constexpr int kMenuInventorySpacing = 10;
constexpr int kMenuPadding = 20;

// Menu title and text font sizes
constexpr float kMainMenuTitleFontSize = 48.0f;
constexpr float kPauseMenuTitleFontSize = 50.0f;
constexpr float kSettingsMenuDescriptionFontSize = 24.0f;
constexpr float kInventoryMenuTitleFontSize = 48.0f;

// Button dimensions and styling
constexpr int kButtonWidth = 220;
constexpr int kButtonHeight = 50;

// Inventory menu constants
constexpr int kInventorySlotSize = 100;
constexpr int kInventoryGridPadding = 10;
constexpr int kInventoryGridSpacing = 15;
constexpr double kInventoryCanvasWidthScale = 0.65; // 65% of canvas width
constexpr int kInventoryBottomOffset = 20;
constexpr const char* kInventoryHighlightColor = "#ffffff73"; // White with ~45% opacity

// Grid rendering constants
constexpr int kVisibleGridWidth = 19;
constexpr int kGridCenterOffset = kVisibleGridWidth / 2; // 9 cells left/right of player

void SetLayoutVisible(WebLayout* layout, bool visible) {
    if (!layout)
        return;
    if (layout->IsVisible() != visible)
        layout->ToggleVisibility();
}
} // namespace

// handler for inventory slot clicks
extern "C" {
EMSCRIPTEN_KEEPALIVE
void WebInterface_handleInventorySlotClick(intptr_t webInterfacePtr, size_t slotIndex) {
    auto* pThis = reinterpret_cast<cse498::WebInterface*>(webInterfacePtr);
    pThis->OnInventorySlotClick(slotIndex);
}
}

using emscripten::val;

static constexpr const char* PLAYER_IMAGE = "/assets/agents/playerCharacter/agent_player.png";
static constexpr const char* MONSTER_IMAGE = "/assets/agents/monsters/agent_monster_goblin.png";
static constexpr const char* INVENTORY_IMAGE = "/assets/gui/inventory_bar.png";
static constexpr const char* EMPTY_SLOT_IMAGE = "/assets/gui/placeholder.png";
static constexpr double kActionInterval = 200.0; // milliseconds between player actions

static constexpr int IMAGE_SIZE = 256;
static constexpr int INVENTORY_IMAGE_WIDTH = 1860;
static constexpr int INVENTORY_IMAGE_HEIGHT = 186;

// Use EM_ASYNC_JS to load images synchronously with await
EM_ASYNC_JS(emscripten::EM_VAL, loadBitmap, (const char* path), {
    try {
        const filename = UTF8ToString(path);

        // Read the file from the sandboxed filesystem
        // This returns a Uint8Array view of the file in the Emscripten heap
        const data = FS.readFile(filename);

        // Create a Blob from the data
        // specify the MIME type in case it can't be detected automatically
        const blob = new Blob([data], {
            type:
                'image/png'
        });

        // Convert to ImageBitmap
        const bitmap = await createImageBitmap(blob);

        return Emval.toHandle(bitmap);
    } catch (e) {
        console.error("Failed to load image from FS:", e);
        return Emval.toHandle(null);
    }
});

// Wrapper to return a proper emscripten::val
val loadImage(const std::string& path) {
    auto handle = loadBitmap(path.c_str());
    return val::take_ownership(handle);
}

WebInterface::WebInterface(std::unique_ptr<InteractiveWorld> overworld, std::unique_ptr<DungeonWorld> dungeonWorld) :
    mInteractiveWorld(std::move(overworld)), mDungeon(std::move(dungeonWorld)), mInputManager(*this) {

    assert(mInteractiveWorld && "InteractiveWorld instance is required");
    assert(mDungeon && "DungeonWorld instance is required");

    // Set up the symbol-to-path map for the interactive world, hardcoded because
    // InteractiveWorld doesn't have a data-driven way to specify these currently
    mSymbolPathOverworld[' '] = "assets/world/forest/floor_tiles/tile_grass_1.png";
    mSymbolPathOverworld['#'] = "assets/world/forest/walls/external/border_top_forest.png";
    mSymbolPathOverworld['L'] = "assets/tiles/lumber_yard.png"; // Lumber Yard
    mSymbolPathOverworld['Q'] = "assets/tiles/quarry.png"; // Quarry
    mSymbolPathOverworld['M'] = "assets/tiles/ore_mine.png"; // Mine
    mSymbolPathOverworld['l'] = "assets/world/forest/floor_tiles/tile_grass_2.png"; // Wood Spawn
    mSymbolPathOverworld['q'] = "assets/world/forest/floor_tiles/tile_grass_5.png"; // Stone Spawn
    mSymbolPathOverworld['m'] = "assets/world/forest/floor_tiles/tile_grass_3.png"; // Metal Spawn

    // Create root layout hooking into "root" div
    mElements.emplace_back(std::make_unique<WebLayout>("root"));
    mRoot = static_cast<WebLayout*>(mElements.back().get());
    mRoot->SetLayoutType(LayoutType::None);

    // Create canvas
    mElements.emplace_back(std::make_unique<WebCanvas>("web-canvas"));
    mCanvas = static_cast<WebCanvas*>(mElements.back().get());
    mCanvas->SetBackgroundColor(kCanvasBg.ToHex());
    mRoot->AddElement(mCanvas);

    // Create points textbox (now used for inventory display)
    auto hudTextPtr = std::make_unique<WebTextbox>();
    mHUDTextbox = hudTextPtr.get();
    mHUDTextbox->SetCanvasPosition(kHUDPositionX, kHUDPositionY);
    mHUDTextbox->SetFontSize(kHUDFontSize);
    mHUDTextbox->SetColor(kDimGray.ToHex());
    mCanvas->AddElement(std::move(hudTextPtr));

    SetupPauseMenu();

    SetupMainMenu();
    SetupSettingsMenu();
    SetupInventoryMenu();

    UpdateLayoutVisibility();

    RenderFrame();
}

void WebInterface::RunFrame(double currentTimeMs) {
    if (mState == WebState::QUIT) {
        return;
    }

    if (mState == WebState::MAIN_MENU) {
        return;
    }

    if (mGameState == WebState::OVERWORLD) {
        // always select an action based on current input, even if paused,
        // to handle unpausing and menu switching
        const char actionChar = SelectAction(mInteractiveWorld->GetGrid());

        if (IsPaused()) {
            return;
        }

        const double deltaTime = currentTimeMs - mLastTimeMs;
        mPlayerTimer += deltaTime;
        mAgentTimer += deltaTime;
        mLastTimeMs = currentTimeMs;

        if (mPlayerTimer >= kActionInterval) {
            mLastActionChar = actionChar;
            if (actionChar != '\0') {
                mPlayerTimer = 0.0;
                auto player = mInteractiveWorld->GetPlayer();
                int result = player->SelectPlayerAction(actionChar);
                mInteractiveWorld->DoAction(*player, result);
                player->SetActionResult(result);
            }
        }

        if (mAgentTimer >= kActionInterval) {
            mAgentTimer = 0.0;
            mInteractiveWorld->RunAgents();
            mInteractiveWorld->UpdateWorld();
        }
    } else {
        // always select an action based on current input, even if paused,
        // to handle unpausing and menu switching
        const char actionChar = SelectAction(mDungeon->GetGrid());

        if (IsPaused()) {
            return;
        }

        const double deltaTime = currentTimeMs - mLastTimeMs;
        mPlayerTimer += deltaTime;
        mAgentTimer += deltaTime;
        mLastTimeMs = currentTimeMs;

        if (mPlayerTimer >= kActionInterval) {
            mLastActionChar = actionChar;
            if (actionChar != '\0') {
                mPlayerTimer = 0.0;
                auto player = mDungeon->GetPlayer();
                int result = player->SelectPlayerAction(actionChar);
                mDungeon->DoAction(*player, result);
                player->SetActionResult(result);
            }
        }

        if (mAgentTimer >= kActionInterval) {
            mAgentTimer = 0.0;
            mDungeon->RunAgents();
        }
    }

    RenderFrame();
}

bool WebInterface::IsPaused() const {
    return mState == WebState::PAUSED || mState == WebState::SETTINGS || mState == WebState::MAIN_MENU ||
           mState == WebState::INVENTORY;
}

WorldBase& WebInterface::GetWorld() const {
    WorldBase* currentWorld = GetCurrentWorld();
    assert(currentWorld && "No active world for current state");
    return *currentWorld;
}

WorldBase* WebInterface::GetCurrentWorld() const {
    switch (mGameState) {
        case WebState::OVERWORLD:
            return mInteractiveWorld.get();
        case WebState::DUNGEON:
            return mDungeon.get();
        default:
            return nullptr;
    }
}

PlayerAgent* WebInterface::GetCurrentPlayer() const {
    WorldBase* world = GetCurrentWorld();
    if (world) {
        return world->GetPlayer();
    }
    return nullptr;
}

std::unordered_map<char, std::string>& WebInterface::GetSymbolPathMap() {
    switch (mGameState) {
        case WebState::OVERWORLD:
            return mSymbolPathOverworld;
        case WebState::DUNGEON:
            return mSymbolPathDungeon;
        default:
            // Should not reach here, but return a reference to avoid null pointer issues
            // In practice, this should only be called when rendering an active world
            static std::unordered_map<char, std::string> empty;
            return empty;
    }
}

std::string WebInterface::GetImagePath(char symbol) {
    if (mGameState == WebState::OVERWORLD) {
        // Check for special symbols
        if (symbol == '@') {
            return PLAYER_IMAGE;
        }
        if (symbol == '*') {
            return MONSTER_IMAGE;
        }

        // Look up symbol in the pre-populated path map
        if (mSymbolPathOverworld.contains(symbol)) {
            return mSymbolPathOverworld.at(symbol);
        }
    } else {
        // Check for special symbols
        if (symbol == '@') {
            return PLAYER_IMAGE;
        }
        if (symbol == '*') {
            return MONSTER_IMAGE;
        }

        return mDungeon->GetImageFile(symbol);
    }

    return "";
}

emscripten::val WebInterface::GetOrLoadBitmap(const std::string& path) {
    // Check if already loaded
    if (mPathBitmaps.contains(path)) {
        return mPathBitmaps.at(path);
    }

    // Load the bitmap and cache it
    auto bitmap = loadImage(path);
    mPathBitmaps.emplace(path, bitmap);
    return bitmap;
}

const char WebInterface::GetLastActionChar() const { return mLastActionChar; }

void WebInterface::UpdateLayoutVisibility() {
    SetLayoutVisible(mMainMenu, mState == WebState::MAIN_MENU);
    SetLayoutVisible(mPauseMenu, mState == WebState::PAUSED);
    SetLayoutVisible(mSettingsMenu, mState == WebState::SETTINGS);
    SetLayoutVisible(mInventoryMenu, mState == WebState::INVENTORY);
    mRoot->Apply();
}

void WebInterface::SetupMainMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("main-menu"));
    mMainMenu = static_cast<WebLayout*>(mElements.back().get());
    mMainMenu->SetLayoutType(LayoutType::Vertical);
    mMainMenu->SetJustification(Justification::Center);
    mMainMenu->SetAlignItems(Alignment::Center);
    mMainMenu->SetSpacing(kMenuMainSpacing);
    mMainMenu->SetPadding(kMenuPadding);
    mMainMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>("Main Menu"));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    titlePtr->SetAlignment(WebTextbox::TextAlign::Center);
    titlePtr->SetFontSize(kMainMenuTitleFontSize);
    titlePtr->SetColor(kDimGray.ToHex());
    titlePtr->MountToLayout(*mMainMenu);

    auto addButton = [this](const std::string& label, std::function<void()> callback) {
        auto button = std::make_unique<WebButton>(label);
        button->SetCallback(std::move(callback));
        button->SetSize(kButtonWidth, kButtonHeight);
        button->SetBackgroundColor(kButtonColorDark.ToHex());
        button->SetTextColor(kDimGray.ToHex());
        button->MountToLayout(*mMainMenu, Alignment::Center);
        mElements.emplace_back(std::move(button));
    };

    addButton("New Game", [this]() { TransitionTo(WebState::OVERWORLD); });
    addButton("Settings", [this]() { TransitionTo(WebState::SETTINGS); });
    addButton("Quit", [this]() { TransitionTo(WebState::QUIT); });
}

void WebInterface::SetupPauseMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("pause-menu"));
    mPauseMenu = static_cast<WebLayout*>(mElements.back().get());
    mPauseMenu->SetLayoutType(LayoutType::Vertical);
    mPauseMenu->SetJustification(Justification::Center);
    mPauseMenu->SetAlignItems(Alignment::Center);
    mPauseMenu->SetSpacing(kMenuPauseSpacing);
    mPauseMenu->SetPadding(kMenuPadding);
    mPauseMenu->ToggleVisibility();
    mPauseMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>("Game Paused"));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    titlePtr->SetAlignment(WebTextbox::TextAlign::Center);
    titlePtr->SetFontSize(kPauseMenuTitleFontSize);
    titlePtr->SetColor(kDimGray.ToHex());
    titlePtr->MountToLayout(*mPauseMenu);

    auto addButton = [this](const std::string& label, std::function<void()> callback) {
        auto button = std::make_unique<WebButton>(label);
        button->SetCallback(std::move(callback));
        button->SetSize(kButtonWidth, kButtonHeight);
        button->SetBackgroundColor(kButtonColorMedium.ToHex());
        button->SetTextColor(kDimGray.ToHex());
        button->MountToLayout(*mPauseMenu, Alignment::Center);
        mElements.emplace_back(std::move(button));
    };

    addButton("Resume", [this]() { Resume(); });
    addButton("Go to Overworld", [this]() { TransitionTo(WebState::OVERWORLD); });
    addButton("Go to Dungeon", [this]() { TransitionTo(WebState::DUNGEON); });
    addButton("Settings", [this]() { TransitionTo(WebState::SETTINGS); });
    addButton("Quit to Main Menu", [this]() { TransitionTo(WebState::MAIN_MENU); });
}

void WebInterface::SetupSettingsMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("settings-menu"));
    mSettingsMenu = static_cast<WebLayout*>(mElements.back().get());
    mSettingsMenu->SetLayoutType(LayoutType::Vertical);
    mSettingsMenu->SetJustification(Justification::Center);
    mSettingsMenu->SetAlignItems(Alignment::Center);
    mSettingsMenu->SetSpacing(kMenuMainSpacing);
    mSettingsMenu->SetPadding(kMenuPadding);
    mSettingsMenu->ToggleVisibility();
    mSettingsMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>("Settings"));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    titlePtr->SetAlignment(WebTextbox::TextAlign::Center);
    titlePtr->SetFontSize(kMainMenuTitleFontSize);
    titlePtr->SetColor(kDimGray.ToHex());
    titlePtr->MountToLayout(*mSettingsMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>("Settings are not available yet."));
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    descPtr->SetAlignment(WebTextbox::TextAlign::Center);
    descPtr->SetFontSize(kSettingsMenuDescriptionFontSize);
    descPtr->SetColor(kDimGray.ToHex());
    descPtr->MountToLayout(*mSettingsMenu);

    auto backButton = std::make_unique<WebButton>("Back");
    backButton->SetCallback([this]() { TransitionTo(mPreviousState); });
    backButton->SetSize(kButtonWidth, kButtonHeight);
    backButton->SetBackgroundColor(kButtonColorMedium.ToHex());
    backButton->SetTextColor(kButtonTextColorWhite.ToHex());
    backButton->MountToLayout(*mSettingsMenu, Alignment::Center);
    mElements.emplace_back(std::move(backButton));
}

void WebInterface::SetupInventoryMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("inventory-menu"));
    mInventoryMenu = static_cast<WebLayout*>(mElements.back().get());
    mInventoryMenu->SetLayoutType(LayoutType::Vertical);
    mInventoryMenu->SetJustification(Justification::Start);
    mInventoryMenu->SetAlignItems(Alignment::Center);
    mInventoryMenu->SetSpacing(kMenuInventorySpacing);
    mInventoryMenu->SetPadding(kMenuPadding);
    mInventoryMenu->ToggleVisibility();
    mInventoryMenu->MountToLayout(*mRoot);

    // Title
    mElements.emplace_back(std::make_unique<WebTextbox>("Inventory"));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    titlePtr->SetAlignment(WebTextbox::TextAlign::Center);
    titlePtr->SetFontSize(kInventoryMenuTitleFontSize);
    titlePtr->SetColor(kDimGray.ToHex());
    titlePtr->MountToLayout(*mInventoryMenu);

    // Create grid layout for inventory items (backpack only, not hotbar)
    mElements.emplace_back(std::make_unique<WebLayout>("inventory-grid"));
    WebLayout* gridLayout = static_cast<WebLayout*>(mElements.back().get());
    gridLayout->SetLayoutType(LayoutType::Grid);
    gridLayout->SetJustification(Justification::Start);
    gridLayout->SetAlignItems(Alignment::Start);
    gridLayout->SetSpacing(kInventoryGridSpacing);
    gridLayout->SetPadding(kInventoryGridPadding);
    gridLayout->MountToLayout(*mInventoryMenu);

    // Create placeholder image elements for each backpack slot (20 slots)
    // Arranged in 5 columns x 4 rows (ITEMS_PER_ROW = 5, BACKPACK_SIZE = 20)
    mInventoryImages.clear();
    for (size_t i = 0; i < Inventory::BACKPACK_SIZE; ++i) {
        mElements.emplace_back(std::make_unique<WebImage>("", "Inventory Slot " + std::to_string(i)));
        WebImage* imagePtr = static_cast<WebImage*>(mElements.back().get());
        imagePtr->SetSize(kInventorySlotSize, kInventorySlotSize);
        imagePtr->SetSource(EMPTY_SLOT_IMAGE);

        // Calculate grid position
        int row = i / Inventory::ITEMS_PER_ROW;
        int col = i % Inventory::ITEMS_PER_ROW;
        imagePtr->SetGridPosition(row, col);

        imagePtr->MountToLayout(*gridLayout, Alignment::None);

        // Calculate the actual inventory slot index (backpack starts at slot 10)
        size_t slotIndex = Inventory::HOTBAR_SIZE + i;

        // Attach click listener using EM_ASM pattern
        EM_ASM(
                {
                    var el = Emval.toValue($0);
                    var webInterfacePtr = $1;
                    var slotIndex = $2;
                    el.addEventListener(
                            "click",
                            function() { Module._WebInterface_handleInventorySlotClick(webInterfacePtr, slotIndex); });
                },
                imagePtr->GetElement().as_handle(), reinterpret_cast<intptr_t>(this), slotIndex);

        mInventoryImages.push_back({imagePtr, slotIndex});
    }

    // Back button
    auto backButton = std::make_unique<WebButton>("Close");
    backButton->SetCallback([this]() { TransitionTo(mPreviousState); });
    backButton->SetSize(kButtonWidth, kButtonHeight);
    backButton->SetBackgroundColor(kButtonColorMedium.ToHex());
    backButton->SetTextColor(kButtonTextColorWhite.ToHex());
    backButton->MountToLayout(*mInventoryMenu, Alignment::Center);
    mElements.emplace_back(std::move(backButton));
}

void WebInterface::PopulateInventoryMenu() {
    // Get player inventory
    auto player = (mGameState == WebState::OVERWORLD) ? mInteractiveWorld->GetPlayer() : mDungeon->GetPlayer();
    if (!player)
        return;

    const Inventory& inventory = player->GetInventory();
    const auto& inventoryArray = inventory.GetInventoryArray();

    // Update image sources for backpack slots (skip hotbar slots 0-9, start from slot 10)
    for (const auto& slotImage: mInventoryImages) {
        const auto& slot = inventoryArray[slotImage.slotIndex];
        WebImage* imagePtr = slotImage.image;

        if (!slot.IsEmpty()) {
            const Item* item = slot.GetItem();
            if (item) {
                const std::string& imagePath = item->GetImagePath();
                imagePtr->SetSource(imagePath);
                imagePtr->Show();
            }
        } else {
            imagePtr->SetSource(EMPTY_SLOT_IMAGE);
            imagePtr->Show();
        }
    }
    RenderHUD(); // Update HUD to reflect any inventory changes (e.g. gold amount)
}

void WebInterface::OpenInventory() {
    if (mState == WebState::INVENTORY) {
        // If already in inventory, close it
        TransitionTo(mPreviousState);
    } else if (mState == WebState::DUNGEON) {
        // Save current state and transition to inventory
        mPreviousState = mState;
        TransitionTo(WebState::INVENTORY);
        PopulateInventoryMenu();
        UpdateLayoutVisibility();
        RenderFrame();
    }
}

void WebInterface::OnInventorySlotClick(size_t slotIndex) {
    auto player = (mGameState == WebState::OVERWORLD) ? mInteractiveWorld->GetPlayer() : mDungeon->GetPlayer();
    if (player) {
        Inventory& inventory = player->GetInventory();
        size_t handSlot = inventory.GetHandSlotIndex();
        if (handSlot != slotIndex) {
            inventory.SwapSlots(handSlot, slotIndex);
            PopulateInventoryMenu();
        }
    }
}

void WebInterface::TransitionTo(WebState newState) {
    if (newState == mState)
        return;

    mPreviousState = mState;
    // Save gameplay state when entering a menu
    if ((newState == WebState::PAUSED || newState == WebState::SETTINGS) &&
        (mState == WebState::OVERWORLD || mState == WebState::DUNGEON)) {

        mGameState = mState;
    } else if (newState == WebState::OVERWORLD || newState == WebState::DUNGEON) {
        mLastTimeMs = 0.0; // Reset timers when starting/resuming gameplay
        mPlayerTimer = 0.0;
        mAgentTimer = 0.0;
        mGameState = newState; // Ensure game state is updated when transitioning directly to gameplay
    }
    mState = newState;
    UpdateLayoutVisibility();
}

void WebInterface::Pause() {
    if (mState == WebState::PAUSED)
        return;
    TransitionTo(WebState::PAUSED);
}

void WebInterface::Resume() {
    if (mPreviousState == WebState::MAIN_MENU) {
        TransitionTo(WebState::MAIN_MENU);
        return;
    }
    // otherwise always return to the saved gameplay state, not the previous state
    TransitionTo(mGameState);
    UpdateLayoutVisibility();
}

void WebInterface::RenderOverworld() {
    PlayerAgent& player = *mInteractiveWorld->GetPlayer();
    auto itemIds = mInteractiveWorld->GetKnownItems(player);
    auto agentIds = mInteractiveWorld->GetKnownAgents(player);
    auto grid = mInteractiveWorld->GetGrid();
    DrawGrid(grid, itemIds, agentIds);
    RenderHUD();
}

void WebInterface::RenderDungeon() {
    PlayerAgent& player = *mDungeon->GetPlayer();
    auto itemIds = mDungeon->GetKnownItems(player);
    auto agentIds = mDungeon->GetKnownAgents(player);
    auto grid = mDungeon->GetGrid();
    DrawGrid(grid, itemIds, agentIds);
    RenderHUD();
}

void WebInterface::RenderHUD() {
    if (mState != WebState::OVERWORLD && mState != WebState::DUNGEON && mState != WebState::INVENTORY) {
        return;
    }

    if (mGameState == WebState::OVERWORLD) {
        // Display the interactive world inventory in the HUD textbox
        std::string inventoryText;
        const auto& items = mInteractiveWorld->GetInventory().GetItems();

        for (const auto& [itemType, amount]: items) {
            inventoryText += std::string(ItemTypeToString(itemType));
            inventoryText += ": ";
            inventoryText += std::to_string(amount);
            inventoryText += "\n";
        }

        if (inventoryText.empty()) {
            inventoryText = "Inventory empty";
        }

        mHUDTextbox->SetText(inventoryText);
    } else {
        int canvasWidth;
        int canvasHeight;
        emscripten_get_canvas_element_size("#web-canvas", &canvasWidth, &canvasHeight);
        const double dpr = emscripten_get_device_pixel_ratio();

        canvasWidth = canvasWidth / dpr;
        canvasHeight = canvasHeight / dpr;

        const double inventoryWidth = kInventoryCanvasWidthScale;
        const double inventoryScale =
                (canvasWidth * inventoryWidth) / INVENTORY_IMAGE_WIDTH; // Scale to 65% of canvas width
        const double itemScale = inventoryScale * INVENTORY_IMAGE_HEIGHT / IMAGE_SIZE;
        const int itemDrawSize = static_cast<int>(IMAGE_SIZE * itemScale);

        mCanvas->DrawTexture(GetOrLoadBitmap(INVENTORY_IMAGE).as_handle(), canvasWidth / 2,
                             canvasHeight - kInventoryBottomOffset, inventoryScale);

        Inventory& inventory = mDungeon->GetPlayer()->GetInventory();
        const auto& array = inventory.GetInventoryArray();
        size_t handIndex = inventory.GetHandSlotIndex();

        int leftOffset = canvasWidth * ((1 - inventoryWidth) / 2) + itemDrawSize / 2;
        for (size_t i = 0; i < inventory.HOTBAR_SIZE; i++) {
            if (i == handIndex) {
                // Draw a highlight around the currently selected item
                mCanvas->DrawRect(leftOffset - itemDrawSize / 2, canvasHeight - kInventoryBottomOffset - itemDrawSize,
                                  itemDrawSize, itemDrawSize, kInventoryHighlightColor);
            }
            std::string imagePath = array[i].GetItem()->GetImagePath();
            if (imagePath.empty()) {
                leftOffset += itemDrawSize;
                continue;
            }
            mCanvas->DrawTexture(GetOrLoadBitmap(imagePath).as_handle(), leftOffset,
                                 canvasHeight - kInventoryBottomOffset, itemScale);
            leftOffset += itemDrawSize;
        }
        mHUDTextbox->SetText(""); // Clear HUD text when in dungeon since inventory is shown on the canvas
    }
}

const char WebInterface::SelectAction(const WorldGrid& grid) {
    auto userAction = mInputManager.GetAction();
    if (userAction == InputManager::ActiveAction::Pause) {
        HandlePause();
        return ACTION_NONE;
    }

    if (mState != WebState::OVERWORLD && mState != WebState::DUNGEON) {
        return ACTION_NONE;
    }

    switch (userAction) {
        case InputManager::ActiveAction::Up:
            return ACTION_UP;

        case InputManager::ActiveAction::Left:
            return ACTION_LEFT;

        case InputManager::ActiveAction::Down:
            return ACTION_DOWN;

        case InputManager::ActiveAction::Right:
            return ACTION_RIGHT;

        case InputManager::ActiveAction::Interact:
            ++mPoints;
            return ACTION_INTERACT;

        case InputManager::ActiveAction::Quit:
            return ACTION_QUIT;

        case InputManager::ActiveAction::None:
        default:
            return ACTION_NONE;
    }
}

void WebInterface::DrawGrid(const WorldGrid& grid, const std::vector<size_t>& itemIds,
                            const std::vector<size_t>& agentIds) {
    int canvasWidth;
    int canvasHeight;
    emscripten_get_canvas_element_size("#web-canvas", &canvasWidth, &canvasHeight);
    const double dpr = emscripten_get_device_pixel_ratio();

    canvasWidth = canvasWidth / dpr;
    canvasHeight = canvasHeight / dpr;

    // Optimize to fit kVisibleGridWidth squares horizontally on the canvas
    const int drawSize = canvasWidth / kVisibleGridWidth;
    const double scale = drawSize / static_cast<double>(IMAGE_SIZE);

    // Calculate how many visible rows we can show
    int kVisibleGridHeight = canvasHeight / drawSize;
    if (canvasHeight % drawSize != 0)
        ++kVisibleGridHeight; // Round up

    // Get player position
    const WorldPosition playerPos = GetCurrentWorld()->GetPlayerPosition();
    const int playerCellX = gsl::narrow_cast<int>(playerPos.CellX());
    const int playerCellY = gsl::narrow_cast<int>(playerPos.CellY());

    // Calculate visible grid bounds centered on player
    const int minX = playerCellX - kGridCenterOffset;
    const int maxX = playerCellX + kGridCenterOffset;
    const int minY = playerCellY - kVisibleGridHeight / 2;
    const int maxY = playerCellY + kVisibleGridHeight / 2;
    const int screenYOffset = (canvasHeight - kVisibleGridHeight * drawSize) / 2; // Center vertically if extra space

    auto CellXToScreenLeft = [&minX, &drawSize](int cellX) {
        return (drawSize * std::abs(cellX - minX) + drawSize / 2);
    };
    auto CellYToScreenTop = [&minY, &drawSize, &screenYOffset](int cellY) {
        return screenYOffset + (drawSize * std::abs(cellY - minY) + drawSize);
    };

    mCanvas->Clear();
    auto& symbolPathMap = GetSymbolPathMap();

    // Only render visible grid cells
    for (int y = std::max(0, minY); y <= maxY; ++y) {
        for (int x = std::max(0, minX); x <= maxX; ++x) {
            if (!grid.IsValid(x, y)) {
                continue;
            }
            char cell = grid.GetSymbol(WorldPosition{x, y});

            // Get the image path for this symbol from the world
            std::string imagePath = GetImagePath(cell);

            // Update or initialize the symbol-to-path map
            if (symbolPathMap.contains(cell)) {
                if (symbolPathMap.at(cell) != imagePath) {
                    symbolPathMap[cell] = imagePath;
                }
            } else {
                symbolPathMap.emplace(cell, imagePath);
            }

            // Skip if no valid image path
            if (imagePath.empty() || !imagePath.contains(".png")) {
                continue;
            }

            // Get or load the bitmap for this path
            auto bitmap = GetOrLoadBitmap(imagePath);
            int left = CellXToScreenLeft(x);
            int top = CellYToScreenTop(y);
            mCanvas->DrawTexture(bitmap.as_handle(), left, top, scale);
        }
    }

    // Render agents (only process if they're within visible bounds)
    for (const auto& agentId: agentIds) {
        const AgentBase& agent = GetCurrentWorld()->GetAgent(agentId);
        WorldPosition pos = agent.GetPosition();
        const int agentCellX = gsl::narrow_cast<int>(pos.CellX());
        const int agentCellY = gsl::narrow_cast<int>(pos.CellY());

        // Skip agents outside the visible grid area
        if (agentCellX < minX || agentCellX > maxX || agentCellY < minY || agentCellY > maxY) {
            continue;
        }

        char agentSymbol = agent.GetSymbol();

        // Get the image path for this agent symbol from the world
        std::string imagePath = GetImagePath(agentSymbol);

        // Update or initialize the symbol-to-path map
        if (symbolPathMap.contains(agentSymbol)) {
            if (symbolPathMap.at(agentSymbol) != imagePath) {
                symbolPathMap[agentSymbol] = imagePath;
            }
        } else {
            symbolPathMap.emplace(agentSymbol, imagePath);
        }

        // Skip if no valid image path
        if (imagePath.empty() || !imagePath.contains(".png")) {
            continue;
        }

        // Get or load the bitmap for this path
        auto agentTexture = GetOrLoadBitmap(imagePath);
        int agentLeft = CellXToScreenLeft(agentCellX);
        int agentTop = CellYToScreenTop(agentCellY);
        mCanvas->DrawTexture(agentTexture.as_handle(), agentLeft, agentTop, scale);
    }

    // Render Player last to ensure they appear on top
    PlayerAgent* player = GetCurrentWorld()->GetPlayer();
    if (player) {
        WorldPosition pos = player->GetLocation().AsWorldPosition();
        const int playerCellX = gsl::narrow_cast<int>(pos.CellX());
        const int playerCellY = gsl::narrow_cast<int>(pos.CellY());
        if (playerCellX >= minX && playerCellX <= maxX && playerCellY >= minY && playerCellY <= maxY) {
            std::string playerImagePath = GetImagePath('@');
            auto playerTexture = GetOrLoadBitmap(playerImagePath);
            int playerLeft = CellXToScreenLeft(playerCellX);
            int playerTop = CellYToScreenTop(playerCellY);
            mCanvas->DrawTexture(playerTexture.as_handle(), playerLeft, playerTop, scale);
        }
    }
}

void WebInterface::RenderFrame() {
    switch (mState) {
        case WebState::OVERWORLD:
            RenderOverworld();
            break;
        case WebState::DUNGEON:
            RenderDungeon();
            break;
        // Paused and settings screens are managed by DOM visibility, so no rendering needed here.
        case WebState::MAIN_MENU:
        case WebState::PAUSED:
        case WebState::SETTINGS:
        case WebState::INVENTORY:
        case WebState::QUIT:
            break;
    }
    mCanvas->RenderFrame();
}

void WebInterface::HandlePause() {
    if (mState == WebState::OVERWORLD || mState == WebState::DUNGEON) {
        Pause();
    } else if (mState == WebState::PAUSED || mState == WebState::SETTINGS || mState == WebState::INVENTORY) {
        Resume();
    }
    RenderFrame();
}

void WebInterface::SetPlayerHotbarIndex(size_t slotIndex) {
    if (mGameState == WebState::OVERWORLD) {
        // auto player = mOverworld->GetPlayer();
        // if (player) {
        //     player->GetInventory().HotBarIndexMove(slotIndex);
        // }
    } else if (mGameState == WebState::DUNGEON) {
        auto player = mDungeon->GetPlayer();
        if (player) {
            player->GetInventory().HotBarIndexMove(slotIndex);
        }
    }
    RenderHUD();
}

#endif
