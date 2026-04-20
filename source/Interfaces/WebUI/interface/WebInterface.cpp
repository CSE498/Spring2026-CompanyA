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
#include <iomanip>
#include <sstream>

using namespace cse498;

namespace {
// Color constants
constexpr cse498::Color kTextPrimary = cse498::Color::FromRGB255(0xf4, 0xf7, 0xfb);
constexpr cse498::Color kTextMuted = cse498::Color::FromRGB255(0x9f, 0xb0, 0xc8);
constexpr cse498::Color kAccent = cse498::Color::FromRGB255(0x6b, 0xe2, 0xff);
constexpr cse498::Color kAccentStrong = cse498::Color::FromRGB255(0x7c, 0xaa, 0xff);
constexpr cse498::Color kCanvasBg = cse498::Color::FromRGB255(0x0c, 0x10, 0x17);
constexpr cse498::Color kPanelBorder = cse498::Color::FromRGB255(0x31, 0x3d, 0x57);
constexpr cse498::Color kButtonColorDark = cse498::Color::FromRGB255(0x1a, 0x23, 0x36);
constexpr cse498::Color kButtonColorMedium = cse498::Color::FromRGB255(0x22, 0x2d, 0x44);
constexpr cse498::Color kButtonTextColorWhite = cse498::Color::FromRGB255(0xf4, 0xf7, 0xfb);
static_assert(kTextPrimary.R() == 0xf4 && kTextPrimary.G() == 0xf7 && kTextPrimary.B() == 0xfb);
static_assert(kTextMuted.R() == 0x9f && kTextMuted.G() == 0xb0 && kTextMuted.B() == 0xc8);
static_assert(kAccent.R() == 0x6b && kAccent.G() == 0xe2 && kAccent.B() == 0xff);
static_assert(kCanvasBg.R() == 0x0c && kCanvasBg.G() == 0x10 && kCanvasBg.B() == 0x17);

// HUD and UI positioning constants
constexpr float kHUDPositionX = 18.0f;
constexpr float kHUDPositionY = 34.0f;
constexpr int kHUDFontSize = 17;

// Menu layout constants
constexpr int kMenuMainSpacing = 18;
constexpr int kMenuPauseSpacing = 14;
constexpr int kMenuInventorySpacing = 14;
constexpr int kMenuPadding = 28;

// Menu title and text font sizes
constexpr float kMenuEyebrowFontSize = 12.0f;
constexpr float kMainMenuTitleFontSize = 46.0f;
constexpr float kPauseMenuTitleFontSize = 46.0f;
constexpr float kSettingsMenuDescriptionFontSize = 18.0f;
constexpr float kInventoryMenuTitleFontSize = 40.0f;
constexpr float kMenuBodyFontSize = 16.0f;

// Button dimensions and styling
constexpr int kButtonWidth = 250;
constexpr int kButtonHeight = 54;

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

void StyleMenuLayout(WebLayout* layout, int width) {
    if (!layout)
        return;
    layout->SetWidth(width);
    layout->SetBorderWidth(1);
    layout->SetBorderColor(kPanelBorder.ToHex());
    layout->SetBorderRadius(26);
    layout->SetBoxShadow("0 28px 90px rgba(0, 0, 0, 0.42)");
}

void StyleMenuTitle(WebTextbox* textbox, const std::string& color, float sizePx) {
    if (!textbox)
        return;
    textbox->SetAlignment(WebTextbox::TextAlign::Center);
    textbox->SetFontFamily("Inter");
    textbox->SetFallbackFontFamily("system-ui, sans-serif");
    textbox->SetFontSize(sizePx);
    textbox->SetLineHeight(sizePx + 4.0f);
    textbox->SetColor(color);
    textbox->SetBold(true);
    textbox->SetMaxWidth(540.0f);
}

void StyleMenuBody(WebTextbox* textbox, float sizePx = kMenuBodyFontSize) {
    if (!textbox)
        return;
    textbox->SetAlignment(WebTextbox::TextAlign::Center);
    textbox->SetFontFamily("Inter");
    textbox->SetFallbackFontFamily("system-ui, sans-serif");
    textbox->SetFontSize(sizePx);
    textbox->SetLineHeight(sizePx + 8.0f);
    textbox->SetColor(kTextMuted.ToHex());
    textbox->SetMaxWidth(540.0f);
}

void StyleMenuButton(WebButton* button, const std::string& backgroundColor) {
    if (!button)
        return;
    button->SetSize(kButtonWidth, kButtonHeight);
    button->SetBackgroundColor(backgroundColor);
    button->SetTextColor(kButtonTextColorWhite.ToHex());
}

std::string BuildOverworldHudText(const InteractiveWorld& overworld) {
    std::ostringstream out;
    const auto& items = overworld.GetInventory().GetItems();

    auto readAmount = [&items](ItemType itemType) {
        auto it = items.find(itemType);
        return it == items.end() ? 0 : it->second;
    };

    out << "OVERWORLD\n";
    out << "[I] Backpack\n";
    out << "[Esc] Pause\n";
    out << "--------------\n";
    out << std::left << std::setw(6) << "Wood" << " " << readAmount(ItemType::Wood) << "\n";
    out << std::left << std::setw(6) << "Stone" << " " << readAmount(ItemType::Stone) << "\n";
    out << std::left << std::setw(6) << "Metal" << " " << readAmount(ItemType::Metal);
    return out.str();
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
    mHUDTextbox->SetFontFamily("ui-monospace");
    mHUDTextbox->SetFallbackFontFamily("SFMono-Regular, Menlo, Monaco, Consolas, monospace");
    mHUDTextbox->SetFontSize(15.0f);
    mHUDTextbox->SetLineHeight(20.0f);
    mHUDTextbox->SetColor(kTextPrimary.ToHex());
    mHUDTextbox->SetBold(true);
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
    StyleMenuLayout(mMainMenu, 520);
    mMainMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>("WEB INTERFACE"));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccent.ToHex(), kMenuEyebrowFontSize);
    eyebrowPtr->MountToLayout(*mMainMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>("Nightfall Command"));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), kMainMenuTitleFontSize);
    titlePtr->MountToLayout(*mMainMenu);

    mElements.emplace_back(
        std::make_unique<WebTextbox>("Explore the overworld, descend into the dungeon, and manage your inventory from one unified interface.")
    );
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(descPtr);
    descPtr->MountToLayout(*mMainMenu);

    auto addButton = [this](const std::string& label, std::function<void()> callback) {
        auto button = std::make_unique<WebButton>(label);
        button->SetCallback(std::move(callback));
        StyleMenuButton(button.get(), kButtonColorDark.ToHex());
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
    StyleMenuLayout(mPauseMenu, 560);
    mPauseMenu->ToggleVisibility();
    mPauseMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>("SESSION PAUSED"));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccent.ToHex(), kMenuEyebrowFontSize);
    eyebrowPtr->MountToLayout(*mPauseMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>("Take a breath."));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), kPauseMenuTitleFontSize);
    titlePtr->MountToLayout(*mPauseMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>("Resume your run, switch worlds, open settings, or return to the title screen."));
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(descPtr);
    descPtr->MountToLayout(*mPauseMenu);

    auto addButton = [this](const std::string& label, std::function<void()> callback) {
        auto button = std::make_unique<WebButton>(label);
        button->SetCallback(std::move(callback));
        StyleMenuButton(button.get(), kButtonColorMedium.ToHex());
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
    StyleMenuLayout(mSettingsMenu, 520);
    mSettingsMenu->ToggleVisibility();
    mSettingsMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>("SETTINGS"));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccentStrong.ToHex(), kMenuEyebrowFontSize);
    eyebrowPtr->MountToLayout(*mSettingsMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>("Interface Settings"));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), 38.0f);
    titlePtr->MountToLayout(*mSettingsMenu);

    mElements.emplace_back(
        std::make_unique<WebTextbox>("This branch does not expose interactive settings yet, but the layout has been prepared for future controls.")
    );
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(descPtr, kSettingsMenuDescriptionFontSize);
    descPtr->MountToLayout(*mSettingsMenu);

    auto backButton = std::make_unique<WebButton>("Back");
    backButton->SetCallback([this]() { TransitionTo(mPreviousState); });
    StyleMenuButton(backButton.get(), kButtonColorMedium.ToHex());
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
    StyleMenuLayout(mInventoryMenu, 780);
    mInventoryMenu->ToggleVisibility();
    mInventoryMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>("BACKPACK"));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccent.ToHex(), kMenuEyebrowFontSize);
    eyebrowPtr->MountToLayout(*mInventoryMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>("Inventory"));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), kInventoryMenuTitleFontSize);
    titlePtr->MountToLayout(*mInventoryMenu);

    mElements.emplace_back(
        std::make_unique<WebTextbox>("Click a backpack slot to swap it with the item currently held in your hotbar hand slot.")
    );
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(descPtr, 15.0f);
    descPtr->MountToLayout(*mInventoryMenu);

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
    StyleMenuButton(backButton.get(), kButtonColorMedium.ToHex());
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
        mHUDTextbox->SetText(BuildOverworldHudText(*mInteractiveWorld));
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
        mHUDTextbox->SetText("DUNGEON\n[E] Interact\n[I] Backpack\n[1-0] Hotbar\n[Esc] Pause");
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
