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
// -----------------------------------------------------------------------------
// Asset path constants
// -----------------------------------------------------------------------------
constexpr const char* kPlayerImagePath = "/assets/agents/playerCharacter/agent_player.png";
constexpr const char* kMonsterImagePath = "/assets/agents/monsters/agent_monster_goblin.png";
constexpr const char* kInventoryBarImagePath = "/assets/gui/inventory_bar.png";
constexpr const char* kEmptySlotImagePath = "/assets/gui/placeholder.png";

constexpr const char* kOverworldGrassTile1Path = "assets/world/forest/floor_tiles/tile_grass_1.png";
constexpr const char* kOverworldBorderTopForestPath = "assets/world/forest/walls/external/border_top_forest.png";
constexpr const char* kOverworldLumberYardPath = "assets/tiles/lumber_yard.png";
constexpr const char* kOverworldQuarryPath = "assets/tiles/quarry.png";
constexpr const char* kOverworldOreMinePath = "assets/tiles/ore_mine.png";
constexpr const char* kOverworldWoodSpawnTilePath = "assets/world/forest/floor_tiles/tile_grass_2.png";
constexpr const char* kOverworldStoneSpawnTilePath = "assets/world/forest/floor_tiles/tile_grass_5.png";
constexpr const char* kOverworldMetalSpawnTilePath = "assets/world/forest/floor_tiles/tile_grass_3.png";

// -----------------------------------------------------------------------------
// Timing constants
// -----------------------------------------------------------------------------
constexpr double kActionIntervalMs = 200.0;

// -----------------------------------------------------------------------------
// Image and texture constants
// -----------------------------------------------------------------------------
constexpr int kTextureSourceImageSizePx = 256;
constexpr int kInventoryBarImageWidthPx = 1860;
constexpr int kInventoryBarImageHeightPx = 186;
constexpr const char* kPngFileExtension = ".png";

// -----------------------------------------------------------------------------
// Color constants
// -----------------------------------------------------------------------------
constexpr cse498::Color kTextPrimary = cse498::Color::FromRGB255(0xf4, 0xf7, 0xfb);
constexpr cse498::Color kTextMuted = cse498::Color::FromRGB255(0x9f, 0xb0, 0xc8);
constexpr cse498::Color kAccent = cse498::Color::FromRGB255(0x6b, 0xe2, 0xff);
constexpr cse498::Color kAccentStrong = cse498::Color::FromRGB255(0x7c, 0xaa, 0xff);
constexpr cse498::Color kCanvasBackground = cse498::Color::FromRGB255(0x0c, 0x10, 0x17);
constexpr cse498::Color kPanelBorder = cse498::Color::FromRGB255(0x31, 0x3d, 0x57);
constexpr cse498::Color kButtonBackgroundDark = cse498::Color::FromRGB255(0x1a, 0x23, 0x36);
constexpr cse498::Color kButtonBackgroundMedium = cse498::Color::FromRGB255(0x22, 0x2d, 0x44);
constexpr cse498::Color kButtonTextWhite = cse498::Color::FromRGB255(0xf4, 0xf7, 0xfb);

static_assert(kTextPrimary.R() == 0xf4 && kTextPrimary.G() == 0xf7 && kTextPrimary.B() == 0xfb);
static_assert(kTextMuted.R() == 0x9f && kTextMuted.G() == 0xb0 && kTextMuted.B() == 0xc8);
static_assert(kAccent.R() == 0x6b && kAccent.G() == 0xe2 && kAccent.B() == 0xff);
static_assert(kCanvasBackground.R() == 0x0c && kCanvasBackground.G() == 0x10 && kCanvasBackground.B() == 0x17);

// -----------------------------------------------------------------------------
// Typography constants
// -----------------------------------------------------------------------------
constexpr const char* kMenuFontFamily = "Inter";
constexpr const char* kMenuFallbackFontFamily = "system-ui, sans-serif";
constexpr const char* kHUDFontFamily = "ui-monospace";
constexpr const char* kHUDFallbackFontFamily = "SFMono-Regular, Menlo, Monaco, Consolas, monospace";

constexpr float kMenuEyebrowFontSizePx = 12.0f;
constexpr float kMainMenuTitleFontSizePx = 46.0f;
constexpr float kPauseMenuTitleFontSizePx = 46.0f;
constexpr float kSettingsMenuTitleFontSizePx = 38.0f;
constexpr float kSettingsMenuDescriptionFontSizePx = 18.0f;
constexpr float kInventoryMenuTitleFontSizePx = 40.0f;
constexpr float kStatsMenuTitleFontSizePx = 38.0f;
constexpr float kMenuBodyFontSizePx = 16.0f;
constexpr float kCompactMenuBodyFontSizePx = 15.0f;
constexpr float kHUDTextFontSizePx = 15.0f;

constexpr float kMenuTitleLineHeightExtraPx = 4.0f;
constexpr float kMenuBodyLineHeightExtraPx = 8.0f;
constexpr float kHUDTextLineHeightPx = 20.0f;
constexpr float kMenuTextMaxWidthPx = 540.0f;

// -----------------------------------------------------------------------------
// HUD positioning constants
// -----------------------------------------------------------------------------
constexpr float kHUDPositionX = 18.0f;
constexpr float kHUDPositionY = 34.0f;

// -----------------------------------------------------------------------------
// Menu layout constants
// -----------------------------------------------------------------------------
constexpr int kMainMenuSpacingPx = 18;
constexpr int kPauseMenuSpacingPx = 14;
constexpr int kSettingsMenuSpacingPx = kMainMenuSpacingPx;
constexpr int kInventoryMenuSpacingPx = 14;
constexpr int kStatsMenuSpacingPx = kPauseMenuSpacingPx;
constexpr int kMenuPaddingPx = 28;

constexpr int kMainMenuWidthPx = 520;
constexpr int kPauseMenuWidthPx = 560;
constexpr int kSettingsMenuWidthPx = 520;
constexpr int kInventoryMenuWidthPx = 780;
constexpr int kStatsMenuWidthPx = 560;

constexpr int kMenuBorderWidthPx = 1;
constexpr int kMenuBorderRadiusPx = 26;
constexpr const char* kMenuBoxShadow = "0 28px 90px rgba(0, 0, 0, 0.42)";

// -----------------------------------------------------------------------------
// Button constants
// -----------------------------------------------------------------------------
constexpr int kMenuButtonWidthPx = 250;
constexpr int kMenuButtonHeightPx = 54;

// -----------------------------------------------------------------------------
// Inventory menu and HUD constants
// -----------------------------------------------------------------------------
constexpr int kInventorySlotSizePx = 100;
constexpr int kInventoryGridPaddingPx = 10;
constexpr int kInventoryGridSpacingPx = 15;
constexpr double kInventoryCanvasWidthScale = 0.65;
constexpr int kInventoryBarBottomOffsetPx = 20;
constexpr const char* kInventorySelectedSlotHighlightColor = "#ffffff73";
constexpr double kFullWidthScale = 1.0;
constexpr double kCenteredPositionRatio = 0.5;

// -----------------------------------------------------------------------------
// Grid rendering constants
// -----------------------------------------------------------------------------
constexpr int kVisibleGridWidth = 19;
constexpr int kGridCenterOffset = kVisibleGridWidth / 2;
constexpr int kGridVerticalCenterDivisor = 2;
constexpr int kScreenCenterOffsetDivisor = 2;

// -----------------------------------------------------------------------------
// UI copy constants
// -----------------------------------------------------------------------------
constexpr const char* kMainMenuEyebrowText = "WEB INTERFACE";
constexpr const char* kMainMenuTitleText = "Nightfall Command";
constexpr const char* kMainMenuDescriptionText =
        "Explore the overworld, descend into the dungeon, and manage your inventory from one unified interface.";

constexpr const char* kPauseMenuEyebrowText = "SESSION PAUSED";
constexpr const char* kPauseMenuTitleText = "Take a breath.";
constexpr const char* kPauseMenuDescriptionText =
        "Resume your run, switch worlds, open settings, or return to the title screen.";

constexpr const char* kSettingsMenuEyebrowText = "SETTINGS";
constexpr const char* kSettingsMenuTitleText = "Interface Settings";
constexpr const char* kSettingsMenuDescriptionText =
        "This branch does not expose interactive settings yet, but the layout has been prepared for future controls.";

constexpr const char* kInventoryMenuEyebrowText = "BACKPACK";
constexpr const char* kInventoryMenuTitleText = "Inventory";
constexpr const char* kInventoryMenuDescriptionText =
        "Click a backpack slot to swap it with the item currently held in your hotbar hand slot.";

constexpr const char* kStatsMenuEyebrowText = "DATA ANALYTICS";
constexpr const char* kStatsMenuTitleText = "Session Stats";
constexpr const char* kStatsMenuEmptyText = "No stats recorded yet.";

constexpr const char* kNewGameButtonText = "New Game";
constexpr const char* kSettingsButtonText = "Settings";
constexpr const char* kQuitButtonText = "Quit";
constexpr const char* kResumeButtonText = "Resume";
constexpr const char* kGoToOverworldButtonText = "Go to Overworld";
constexpr const char* kGoToDungeonButtonText = "Go to Dungeon";
constexpr const char* kStatsButtonText = "Stats";
constexpr const char* kQuitToMainMenuButtonText = "Quit to Main Menu";
constexpr const char* kBackButtonText = "Back";
constexpr const char* kCloseButtonText = "Close";

constexpr const char* kDungeonHudText = "DUNGEON\n[E] Interact\n[I] Backpack\n[1-0] Hotbar\n[Esc] Pause";

void SetLayoutVisible(WebLayout* layout, bool visible) {
    if (!layout)
        return;
    if (layout->IsVisible() != visible)
        layout->ToggleVisibility();
}

void StyleMenuLayout(WebLayout* layout, int widthPx) {
    if (!layout)
        return;
    layout->SetWidth(widthPx);
    layout->SetBorderWidth(kMenuBorderWidthPx);
    layout->SetBorderColor(kPanelBorder.ToHex());
    layout->SetBorderRadius(kMenuBorderRadiusPx);
    layout->SetBoxShadow(kMenuBoxShadow);
}

void StyleMenuTitle(WebTextbox* textbox, const std::string& color, float sizePx) {
    if (!textbox)
        return;
    textbox->SetAlignment(WebTextbox::TextAlign::Center);
    textbox->SetFontFamily(kMenuFontFamily);
    textbox->SetFallbackFontFamily(kMenuFallbackFontFamily);
    textbox->SetFontSize(sizePx);
    textbox->SetLineHeight(sizePx + kMenuTitleLineHeightExtraPx);
    textbox->SetColor(color);
    textbox->SetBold(true);
    textbox->SetMaxWidth(kMenuTextMaxWidthPx);
}

void StyleMenuBody(WebTextbox* textbox, float sizePx = kMenuBodyFontSizePx) {
    if (!textbox)
        return;
    textbox->SetAlignment(WebTextbox::TextAlign::Center);
    textbox->SetFontFamily(kMenuFontFamily);
    textbox->SetFallbackFontFamily(kMenuFallbackFontFamily);
    textbox->SetFontSize(sizePx);
    textbox->SetLineHeight(sizePx + kMenuBodyLineHeightExtraPx);
    textbox->SetColor(kTextMuted.ToHex());
    textbox->SetMaxWidth(kMenuTextMaxWidthPx);
}

void StyleMenuButton(WebButton* button, const std::string& backgroundColor) {
    if (!button)
        return;
    button->SetSize(kMenuButtonWidthPx, kMenuButtonHeightPx);
    button->SetBackgroundColor(backgroundColor);
    button->SetTextColor(kButtonTextWhite.ToHex());
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
    mSymbolPathOverworld[' '] = kOverworldGrassTile1Path;
    mSymbolPathOverworld['#'] = kOverworldBorderTopForestPath;
    mSymbolPathOverworld['L'] = kOverworldLumberYardPath;
    mSymbolPathOverworld['Q'] = kOverworldQuarryPath;
    mSymbolPathOverworld['M'] = kOverworldOreMinePath;
    mSymbolPathOverworld['l'] = kOverworldWoodSpawnTilePath;
    mSymbolPathOverworld['q'] = kOverworldStoneSpawnTilePath;
    mSymbolPathOverworld['m'] = kOverworldMetalSpawnTilePath;

    // Create root layout hooking into "root" div
    mElements.emplace_back(std::make_unique<WebLayout>("root"));
    mRoot = static_cast<WebLayout*>(mElements.back().get());
    mRoot->SetLayoutType(LayoutType::None);

    // Create canvas
    mElements.emplace_back(std::make_unique<WebCanvas>("web-canvas"));
    mCanvas = static_cast<WebCanvas*>(mElements.back().get());
    mCanvas->SetBackgroundColor(kCanvasBackground.ToHex());
    mRoot->AddElement(mCanvas);

    // Create points textbox (now used for inventory display)
    auto hudTextPtr = std::make_unique<WebTextbox>();
    mHUDTextbox = hudTextPtr.get();
    mHUDTextbox->SetCanvasPosition(kHUDPositionX, kHUDPositionY);
    mHUDTextbox->SetFontFamily(kHUDFontFamily);
    mHUDTextbox->SetFallbackFontFamily(kHUDFallbackFontFamily);
    mHUDTextbox->SetFontSize(kHUDTextFontSizePx);
    mHUDTextbox->SetLineHeight(kHUDTextLineHeightPx);
    mHUDTextbox->SetColor(kTextPrimary.ToHex());
    mHUDTextbox->SetBold(true);
    mCanvas->AddElement(std::move(hudTextPtr));

    SetupPauseMenu();

    SetupMainMenu();
    SetupSettingsMenu();
    SetupInventoryMenu();

    SetupStatsMenu();
    mAnalyticsManager = std::make_shared<AnalyticsManager>();
    mStatsTracker = std::make_unique<StatsTracker>();

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

        if (mPlayerTimer >= kActionIntervalMs) {
            mLastActionChar = actionChar;
            if (actionChar != '\0') {
                mPlayerTimer = 0.0;
                auto player = mInteractiveWorld->GetPlayer();
                int result = player->SelectPlayerAction(actionChar);
                mInteractiveWorld->DoAction(*player, result);
                player->SetActionResult(result);
            }
        }

        if (mAgentTimer >= kActionIntervalMs) {
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

        if (mPlayerTimer >= kActionIntervalMs) {
            mLastActionChar = actionChar;
            if (actionChar != '\0') {
                mPlayerTimer = 0.0;
                auto player = mDungeon->GetPlayer();
                int result = player->SelectPlayerAction(actionChar);
                mDungeon->DoAction(*player, result);
                player->SetActionResult(result);
            }
        }

        if (mAgentTimer >= kActionIntervalMs) {
            mAgentTimer = 0.0;
            mDungeon->RunAgents();
        }
    }

    RenderFrame();
}

bool WebInterface::IsPaused() const {
    return mState == WebState::PAUSED || mState == WebState::SETTINGS || mState == WebState::MAIN_MENU ||
           mState == WebState::INVENTORY || mState == WebState::STATS;
}

/// Returns the active gameplay world.
/// Precondition: an active gameplay world must exist.
/// Use GetCurrentWorld() instead when the caller may be in a menu-only state.
WorldBase& WebInterface::GetWorld() const {
    WorldBase* currentWorld = GetCurrentWorld();
    assert(currentWorld && "GetWorld() is only valid when an active gameplay world exists");
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
            return kPlayerImagePath;
        }
        if (symbol == '*') {
            return kMonsterImagePath;
        }

        // Look up symbol in the pre-populated path map
        if (mSymbolPathOverworld.contains(symbol)) {
            return mSymbolPathOverworld.at(symbol);
        }
    } else {
        // Check for special symbols
        if (symbol == '@') {
            return kPlayerImagePath;
        }
        if (symbol == '*') {
            return kMonsterImagePath;
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
    SetLayoutVisible(mStatsMenu, mState == WebState::STATS);
    mRoot->Apply();
}

void WebInterface::SetupMainMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("main-menu"));
    mMainMenu = static_cast<WebLayout*>(mElements.back().get());
    mMainMenu->SetLayoutType(LayoutType::Vertical);
    mMainMenu->SetJustification(Justification::Center);
    mMainMenu->SetAlignItems(Alignment::Center);
    mMainMenu->SetSpacing(kMainMenuSpacingPx);
    mMainMenu->SetPadding(kMenuPaddingPx);
    StyleMenuLayout(mMainMenu, kMainMenuWidthPx);
    mMainMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>(kMainMenuEyebrowText));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccent.ToHex(), kMenuEyebrowFontSizePx);
    eyebrowPtr->MountToLayout(*mMainMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kMainMenuTitleText));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), kMainMenuTitleFontSizePx);
    titlePtr->MountToLayout(*mMainMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kMainMenuDescriptionText));
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(descPtr);
    descPtr->MountToLayout(*mMainMenu);

    auto addButton = [this](const std::string& label, std::function<void()> callback) {
        auto button = std::make_unique<WebButton>(label);
        button->SetCallback(std::move(callback));
        StyleMenuButton(button.get(), kButtonBackgroundDark.ToHex());
        button->MountToLayout(*mMainMenu, Alignment::Center);
        mElements.emplace_back(std::move(button));
    };

    addButton(kNewGameButtonText, [this]() { TransitionTo(WebState::OVERWORLD); });
    addButton(kSettingsButtonText, [this]() { TransitionTo(WebState::SETTINGS); });
    addButton(kQuitButtonText, [this]() { TransitionTo(WebState::QUIT); });
}

void WebInterface::SetupPauseMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("pause-menu"));
    mPauseMenu = static_cast<WebLayout*>(mElements.back().get());
    mPauseMenu->SetLayoutType(LayoutType::Vertical);
    mPauseMenu->SetJustification(Justification::Center);
    mPauseMenu->SetAlignItems(Alignment::Center);
    mPauseMenu->SetSpacing(kPauseMenuSpacingPx);
    mPauseMenu->SetPadding(kMenuPaddingPx);
    StyleMenuLayout(mPauseMenu, kPauseMenuWidthPx);
    mPauseMenu->ToggleVisibility();
    mPauseMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>(kPauseMenuEyebrowText));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccent.ToHex(), kMenuEyebrowFontSizePx);
    eyebrowPtr->MountToLayout(*mPauseMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kPauseMenuTitleText));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), kPauseMenuTitleFontSizePx);
    titlePtr->MountToLayout(*mPauseMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kPauseMenuDescriptionText));
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(descPtr);
    descPtr->MountToLayout(*mPauseMenu);

    auto addButton = [this](const std::string& label, std::function<void()> callback) {
        auto button = std::make_unique<WebButton>(label);
        button->SetCallback(std::move(callback));
        StyleMenuButton(button.get(), kButtonBackgroundMedium.ToHex());
        button->MountToLayout(*mPauseMenu, Alignment::Center);
        mElements.emplace_back(std::move(button));
    };

    addButton(kResumeButtonText, [this]() { Resume(); });
    addButton(kGoToOverworldButtonText, [this]() { TransitionTo(WebState::OVERWORLD); });
    addButton(kGoToDungeonButtonText, [this]() { TransitionTo(WebState::DUNGEON); });
    addButton(kSettingsButtonText, [this]() { TransitionTo(WebState::SETTINGS); });
    addButton(kStatsButtonText, [this]() {
        if (mAnalyticsManager && mStatsTracker) {
            mDashboardSnapshot = mStatsTracker->BuildSnapshot(*mAnalyticsManager);
        }
        TransitionTo(WebState::STATS);
    });
    addButton(kQuitToMainMenuButtonText, [this]() { TransitionTo(WebState::MAIN_MENU); });
}

void WebInterface::SetupSettingsMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("settings-menu"));
    mSettingsMenu = static_cast<WebLayout*>(mElements.back().get());
    mSettingsMenu->SetLayoutType(LayoutType::Vertical);
    mSettingsMenu->SetJustification(Justification::Center);
    mSettingsMenu->SetAlignItems(Alignment::Center);
    mSettingsMenu->SetSpacing(kSettingsMenuSpacingPx);
    mSettingsMenu->SetPadding(kMenuPaddingPx);
    StyleMenuLayout(mSettingsMenu, kSettingsMenuWidthPx);
    mSettingsMenu->ToggleVisibility();
    mSettingsMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>(kSettingsMenuEyebrowText));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccentStrong.ToHex(), kMenuEyebrowFontSizePx);
    eyebrowPtr->MountToLayout(*mSettingsMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kSettingsMenuTitleText));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), kSettingsMenuTitleFontSizePx);
    titlePtr->MountToLayout(*mSettingsMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kSettingsMenuDescriptionText));
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(descPtr, kSettingsMenuDescriptionFontSizePx);
    descPtr->MountToLayout(*mSettingsMenu);

    auto backButton = std::make_unique<WebButton>(kBackButtonText);
    backButton->SetCallback([this]() { TransitionTo(mPreviousState); });
    StyleMenuButton(backButton.get(), kButtonBackgroundMedium.ToHex());
    backButton->MountToLayout(*mSettingsMenu, Alignment::Center);
    mElements.emplace_back(std::move(backButton));
}

void WebInterface::SetupInventoryMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("inventory-menu"));
    mInventoryMenu = static_cast<WebLayout*>(mElements.back().get());
    mInventoryMenu->SetLayoutType(LayoutType::Vertical);
    mInventoryMenu->SetJustification(Justification::Start);
    mInventoryMenu->SetAlignItems(Alignment::Center);
    mInventoryMenu->SetSpacing(kInventoryMenuSpacingPx);
    mInventoryMenu->SetPadding(kMenuPaddingPx);
    StyleMenuLayout(mInventoryMenu, kInventoryMenuWidthPx);
    mInventoryMenu->ToggleVisibility();
    mInventoryMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>(kInventoryMenuEyebrowText));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccent.ToHex(), kMenuEyebrowFontSizePx);
    eyebrowPtr->MountToLayout(*mInventoryMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kInventoryMenuTitleText));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), kInventoryMenuTitleFontSizePx);
    titlePtr->MountToLayout(*mInventoryMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kInventoryMenuDescriptionText));
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(descPtr, kCompactMenuBodyFontSizePx);
    descPtr->MountToLayout(*mInventoryMenu);

    // Create grid layout for inventory items (backpack only, not hotbar)
    mElements.emplace_back(std::make_unique<WebLayout>("inventory-grid"));
    WebLayout* gridLayout = static_cast<WebLayout*>(mElements.back().get());
    gridLayout->SetLayoutType(LayoutType::Grid);
    gridLayout->SetJustification(Justification::Start);
    gridLayout->SetAlignItems(Alignment::Start);
    gridLayout->SetSpacing(kInventoryGridSpacingPx);
    gridLayout->SetPadding(kInventoryGridPaddingPx);
    gridLayout->MountToLayout(*mInventoryMenu);

    // Create placeholder image elements for each backpack slot.
    // Arranged by Inventory::ITEMS_PER_ROW and Inventory::BACKPACK_SIZE.
    mInventoryImages.clear();
    for (size_t i = 0; i < Inventory::BACKPACK_SIZE; ++i) {
        mElements.emplace_back(std::make_unique<WebImage>("", "Inventory Slot " + std::to_string(i)));
        WebImage* imagePtr = static_cast<WebImage*>(mElements.back().get());
        imagePtr->SetSize(kInventorySlotSizePx, kInventorySlotSizePx);
        imagePtr->SetSource(kEmptySlotImagePath);

        const int row = i / Inventory::ITEMS_PER_ROW;
        const int col = i % Inventory::ITEMS_PER_ROW;
        imagePtr->SetGridPosition(row, col);

        imagePtr->MountToLayout(*gridLayout, Alignment::None);

        const size_t slotIndex = Inventory::HOTBAR_SIZE + i;

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

    auto backButton = std::make_unique<WebButton>(kCloseButtonText);
    backButton->SetCallback([this]() { TransitionTo(mPreviousState); });
    StyleMenuButton(backButton.get(), kButtonBackgroundMedium.ToHex());
    backButton->MountToLayout(*mInventoryMenu, Alignment::Center);
    mElements.emplace_back(std::move(backButton));
}

void WebInterface::SetupStatsMenu() {
    mElements.emplace_back(std::make_unique<WebLayout>("stats-menu"));
    mStatsMenu = static_cast<WebLayout*>(mElements.back().get());
    mStatsMenu->SetLayoutType(LayoutType::Vertical);
    mStatsMenu->SetJustification(Justification::Start);
    mStatsMenu->SetAlignItems(Alignment::Center);
    mStatsMenu->SetSpacing(kStatsMenuSpacingPx);
    mStatsMenu->SetPadding(kMenuPaddingPx);
    StyleMenuLayout(mStatsMenu, kStatsMenuWidthPx);
    mStatsMenu->ToggleVisibility();
    mStatsMenu->MountToLayout(*mRoot);

    mElements.emplace_back(std::make_unique<WebTextbox>(kStatsMenuEyebrowText));
    WebTextbox* eyebrowPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(eyebrowPtr, kAccent.ToHex(), kMenuEyebrowFontSizePx);
    eyebrowPtr->MountToLayout(*mStatsMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kStatsMenuTitleText));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuTitle(titlePtr, kTextPrimary.ToHex(), kStatsMenuTitleFontSizePx);
    titlePtr->MountToLayout(*mStatsMenu);

    mElements.emplace_back(std::make_unique<WebTextbox>(kStatsMenuEmptyText));
    WebTextbox* bodyPtr = static_cast<WebTextbox*>(mElements.back().get());
    StyleMenuBody(bodyPtr, kCompactMenuBodyFontSizePx);
    bodyPtr->MountToLayout(*mStatsMenu);

    auto backButton = std::make_unique<WebButton>(kBackButtonText);
    backButton->SetCallback([this]() { TransitionTo(mPreviousState); });
    StyleMenuButton(backButton.get(), kButtonBackgroundMedium.ToHex());
    backButton->MountToLayout(*mStatsMenu, Alignment::Center);
    mElements.emplace_back(std::move(backButton));
}

void WebInterface::PopulateInventoryMenu() {
    auto player = (mGameState == WebState::OVERWORLD) ? mInteractiveWorld->GetPlayer() : mDungeon->GetPlayer();
    if (!player)
        return;

    const Inventory& inventory = player->GetInventory();
    const auto& inventoryArray = inventory.GetInventoryArray();

    // Update image sources for backpack slots.
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
            imagePtr->SetSource(kEmptySlotImagePath);
            imagePtr->Show();
        }
    }
    RenderHUD();
}

void WebInterface::OpenInventory() {
    if (mState == WebState::INVENTORY) {
        TransitionTo(mPreviousState);
    } else if (mState == WebState::DUNGEON) {
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

    if ((newState == WebState::PAUSED || newState == WebState::SETTINGS || newState == WebState::STATS) &&
        (mState == WebState::OVERWORLD || mState == WebState::DUNGEON)) {

        mGameState = mState;
    } else if (newState == WebState::OVERWORLD || newState == WebState::DUNGEON) {
        mLastTimeMs = 0.0;
        mPlayerTimer = 0.0;
        mAgentTimer = 0.0;
        mGameState = newState;
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
                (canvasWidth * inventoryWidth) / kInventoryBarImageWidthPx;
        const double itemScale = inventoryScale * kInventoryBarImageHeightPx / kTextureSourceImageSizePx;
        const int itemDrawSize = static_cast<int>(kTextureSourceImageSizePx * itemScale);

        mCanvas->DrawTexture(GetOrLoadBitmap(kInventoryBarImagePath).as_handle(),
                             canvasWidth * kCenteredPositionRatio,
                             canvasHeight - kInventoryBarBottomOffsetPx,
                             inventoryScale);

        Inventory& inventory = mDungeon->GetPlayer()->GetInventory();
        const auto& array = inventory.GetInventoryArray();
        size_t handIndex = inventory.GetHandSlotIndex();

        int leftOffset = canvasWidth * ((kFullWidthScale - inventoryWidth) / 2) + itemDrawSize / 2;
        for (size_t i = 0; i < inventory.HOTBAR_SIZE; i++) {
            if (i == handIndex) {
                mCanvas->DrawRect(leftOffset - itemDrawSize / 2,
                                  canvasHeight - kInventoryBarBottomOffsetPx - itemDrawSize,
                                  itemDrawSize,
                                  itemDrawSize,
                                  kInventorySelectedSlotHighlightColor);
            }

            const Item* item = array[i].GetItem();
            if (!item) {
                leftOffset += itemDrawSize;
                continue;
            }

            std::string imagePath = item->GetImagePath();
            if (imagePath.empty()) {
                leftOffset += itemDrawSize;
                continue;
            }

            mCanvas->DrawTexture(GetOrLoadBitmap(imagePath).as_handle(),
                                 leftOffset,
                                 canvasHeight - kInventoryBarBottomOffsetPx,
                                 itemScale);
            leftOffset += itemDrawSize;
        }

        mHUDTextbox->SetText(kDungeonHudText);
    }
}

const char WebInterface::SelectAction(const WorldGrid& grid) {
    auto userAction = mInputManager.GetAction();
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
            return ACTION_INTERACT;

        case InputManager::ActiveAction::Quit:
            return ACTION_QUIT;

        case InputManager::ActiveAction::None:
        default:
            return ACTION_NONE;
    }
}

void WebInterface::DrawGrid(const WorldGrid& grid,
                            const std::vector<size_t>& itemIds,
                            const std::vector<size_t>& agentIds) {
    int canvasWidth;
    int canvasHeight;
    emscripten_get_canvas_element_size("#web-canvas", &canvasWidth, &canvasHeight);
    const double dpr = emscripten_get_device_pixel_ratio();

    canvasWidth = canvasWidth / dpr;
    canvasHeight = canvasHeight / dpr;

    const int drawSize = canvasWidth / kVisibleGridWidth;
    const double scale = drawSize / static_cast<double>(kTextureSourceImageSizePx);

    int visibleGridHeight = canvasHeight / drawSize;
    if (canvasHeight % drawSize != 0)
        ++visibleGridHeight;

    const WorldPosition playerPos = GetCurrentWorld()->GetPlayerPosition();
    const int playerCellX = gsl::narrow_cast<int>(playerPos.CellX());
    const int playerCellY = gsl::narrow_cast<int>(playerPos.CellY());

    const int minX = playerCellX - kGridCenterOffset;
    const int maxX = playerCellX + kGridCenterOffset;
    const int minY = playerCellY - visibleGridHeight / kGridVerticalCenterDivisor;
    const int maxY = playerCellY + visibleGridHeight / kGridVerticalCenterDivisor;
    const int screenYOffset =
            (canvasHeight - visibleGridHeight * drawSize) / kScreenCenterOffsetDivisor;

    auto CellXToScreenLeft = [&minX, &drawSize](int cellX) {
        return (drawSize * std::abs(cellX - minX) + drawSize / 2);
    };
    auto CellYToScreenTop = [&minY, &drawSize, &screenYOffset](int cellY) {
        return screenYOffset + (drawSize * std::abs(cellY - minY) + drawSize);
    };

    mCanvas->Clear();
    auto& symbolPathMap = GetSymbolPathMap();

    for (int y = std::max(0, minY); y <= maxY; ++y) {
        for (int x = std::max(0, minX); x <= maxX; ++x) {
            if (!grid.IsValid(x, y)) {
                continue;
            }

            char cell = grid.GetSymbol(WorldPosition{x, y});
            std::string imagePath = GetImagePath(cell);

            if (symbolPathMap.contains(cell)) {
                if (symbolPathMap.at(cell) != imagePath) {
                    symbolPathMap[cell] = imagePath;
                }
            } else {
                symbolPathMap.emplace(cell, imagePath);
            }

            if (imagePath.empty() || !imagePath.contains(kPngFileExtension)) {
                continue;
            }

            auto bitmap = GetOrLoadBitmap(imagePath);
            int left = CellXToScreenLeft(x);
            int top = CellYToScreenTop(y);
            mCanvas->DrawTexture(bitmap.as_handle(), left, top, scale);
        }
    }

    for (const auto& agentId: agentIds) {
        const AgentBase& agent = GetCurrentWorld()->GetAgent(agentId);
        WorldPosition pos = agent.GetPosition();
        const int agentCellX = gsl::narrow_cast<int>(pos.CellX());
        const int agentCellY = gsl::narrow_cast<int>(pos.CellY());

        if (agentCellX < minX || agentCellX > maxX || agentCellY < minY || agentCellY > maxY) {
            continue;
        }

        char agentSymbol = agent.GetSymbol();
        std::string imagePath = GetImagePath(agentSymbol);

        if (symbolPathMap.contains(agentSymbol)) {
            if (symbolPathMap.at(agentSymbol) != imagePath) {
                symbolPathMap[agentSymbol] = imagePath;
            }
        } else {
            symbolPathMap.emplace(agentSymbol, imagePath);
        }

        if (imagePath.empty() || !imagePath.contains(kPngFileExtension)) {
            continue;
        }

        auto agentTexture = GetOrLoadBitmap(imagePath);
        int agentLeft = CellXToScreenLeft(agentCellX);
        int agentTop = CellYToScreenTop(agentCellY);
        mCanvas->DrawTexture(agentTexture.as_handle(), agentLeft, agentTop, scale);
    }

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
        case WebState::MAIN_MENU:
        case WebState::PAUSED:
        case WebState::SETTINGS:
        case WebState::INVENTORY:
        case WebState::STATS:
        case WebState::QUIT:
            break;
    }
    mCanvas->RenderFrame();
}

void WebInterface::HandlePause() {
    if (mState == WebState::OVERWORLD || mState == WebState::DUNGEON) {
        Pause();
    } else if (mState == WebState::PAUSED ||
               mState == WebState::SETTINGS ||
               mState == WebState::INVENTORY ||
               mState == WebState::STATS) {
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
