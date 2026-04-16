#ifdef __EMSCRIPTEN__

#include "./WebInterface.hpp"
#include "../../../../third-party/gsl/gsl"
#include "../../../core/WorldBase.hpp"
#include "../../../tools/Color.hpp"
#include "../WebButton/WebButton.hpp"
#include "../WebCanvas/WebCanvas.hpp"
#include "../WebLayout/WebLayout.hpp"
#include "../WebTextbox/WebTextbox.hpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <sstream>

using namespace cse498;

namespace {
    constexpr cse498::Color kDimGray = cse498::Color::FromRGB255(0xcc, 0xcc, 0xcc);
    constexpr cse498::Color kCanvasBg = cse498::Color::FromRGB255(0x0c, 0x10, 0x17);
    static_assert(kDimGray.R() == 0xcc && kDimGray.G() == 0xcc && kDimGray.B() == 0xcc);
    static_assert(kCanvasBg.R() == 0x0c && kCanvasBg.G() == 0x10 && kCanvasBg.B() == 0x17);
} // namespace

using emscripten::val;

static constexpr const char* PLAYER_IMAGE = "agents/playerCharacter/agent_player.png";
static constexpr const char* MONSTER_IMAGE = "agents/monsters/agent_monster_goblin.png";

static constexpr int IMAGE_SIZE = 256;

// Use EM_ASYNC_JS to load images synchronously with await
EM_ASYNC_JS(emscripten::EM_VAL, loadBitmap, (const char* path), {
    try {
        const filename = "/assets/" + UTF8ToString(path);

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

WebInterface::WebInterface(size_t id, const std::string& name, const WorldBase& world) :
    InterfaceBase(id, name, world) {
    // Create root layout hooking into "root" div
    mElements.emplace_back(std::make_unique<WebLayout>("root"));
    mRoot = static_cast<WebLayout*>(mElements.back().get());
    mRoot->SetLayoutType(LayoutType::None);

    // Create canvas
    mElements.emplace_back(std::make_unique<WebCanvas>("web-canvas"));
    mCanvas = static_cast<WebCanvas*>(mElements.back().get());
    mCanvas->SetBackgroundColor(kCanvasBg.ToHex());
    mRoot->AddElement(mCanvas);

    // Create points textbox
    auto pointsTextPtr = std::make_unique<WebTextbox>();
    mPointsTextbox = pointsTextPtr.get();
    mPointsTextbox->SetCanvasPosition(10.0f, 50.0f);
    mPointsTextbox->SetFontSize(24);
    mPointsTextbox->SetColor(kDimGray.ToHex());
    mCanvas->AddElement(std::move(pointsTextPtr));

    // Create pause menu layout
    mElements.emplace_back(std::make_unique<WebLayout>("pause-menu"));
    mPauseMenu = static_cast<WebLayout*>(mElements.back().get());
    mPauseMenu->SetLayoutType(LayoutType::Vertical);
    mPauseMenu->SetJustification(Justification::Center);
    mPauseMenu->SetAlignItems(Alignment::Center);
    mPauseMenu->SetSpacing(10);
    mPauseMenu->ToggleVisibility();
    mPauseMenu->MountToLayout(*mRoot);

    // Create title textbox
    mElements.emplace_back(std::make_unique<WebTextbox>("Game Paused"));
    WebTextbox* titlePtr = static_cast<WebTextbox*>(mElements.back().get());
    titlePtr->SetAlignment(WebTextbox::TextAlign::Center);
    titlePtr->SetFontSize(50.0f);
    titlePtr->SetColor(kDimGray.ToHex());
    titlePtr->MountToLayout(*mPauseMenu);

    // Create description textbox
    mElements.emplace_back(std::make_unique<WebTextbox>("press Escape to resume, or Q to quit."));
    WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
    descPtr->SetAlignment(WebTextbox::TextAlign::Center);
    descPtr->SetFontSize(24.0f);
    descPtr->SetColor(kDimGray.ToHex());
    descPtr->MountToLayout(*mPauseMenu);

    auto cellTypes = world.GetGrid().GetCellTypes();

    std::ranges::for_each(cellTypes, [this](const CellType& cell) {
        if (cell.name == "Unknown")
            return;
        mTextures.emplace(cell.symbol, loadImage(cell.desc));
    });

    mTextures.emplace('@', loadImage(PLAYER_IMAGE));
    mTextures.emplace('*', loadImage(MONSTER_IMAGE));

    for (const auto& cell: cellTypes) {
        if (cell.name == "Unknown")
            continue;
        assert(mTextures.contains(cell.symbol) &&
               (std::string("Missing texture for cell symbol: ") + cell.symbol).c_str());
    }

    assert(mTextures.contains('@') && "Missing texture for player symbol: @");
    assert(mTextures.contains('*') && "Missing texture for monster symbol: *");

    RenderFrame();
}

size_t WebInterface::SelectAction(const WorldGrid& grid) {
    auto userAction = mInputManager.GetAction();

    if (mPaused && userAction != InputManager::ActiveAction::Pause && userAction != InputManager::ActiveAction::Quit)
        return 0;

    // mActionMap is populated with all available actions in ConfigAgent()
    // and all these values are checked in Initialize();
    // switch (userAction) {
    //   case InputManager::ActiveAction::Up:       return mActionMap.at("up");
    //   case InputManager::ActiveAction::Left:     return mActionMap.at("left");
    //   case InputManager::ActiveAction::Down:     return mActionMap.at("down");
    //   case InputManager::ActiveAction::Right:    return mActionMap.at("right");
    //   case InputManager::ActiveAction::Interact: ++mPoints; return mActionMap.at("interact");
    //   case InputManager::ActiveAction::Quit:     return mActionMap.at("quit");
    //   case InputManager::ActiveAction::Pause:    return 0;
    //   case InputManager::ActiveAction::None:     return 0;
    //   default: return 0;
    // }

    // KAREN: Modified since things changed in AgentBase
    switch (userAction) {
        case InputManager::ActiveAction::Up:
            return GetActionID("up");

        case InputManager::ActiveAction::Left:
            return GetActionID("left");

        case InputManager::ActiveAction::Down:
            return GetActionID("down");

        case InputManager::ActiveAction::Right:
            return GetActionID("right");

        case InputManager::ActiveAction::Interact:
            ++mPoints;
            return GetActionID("interact");

        case InputManager::ActiveAction::Quit:
            return GetActionID("quit");

        case InputManager::ActiveAction::Pause:
        case InputManager::ActiveAction::None:
        default:
            return 0;
    }
}

bool WebInterface::Initialize() {
    std::array<const char*, 6> actions{"up", "left", "down", "right", "interact", "quit"};

    // true if any of these actions are not available
    bool invalid = std::ranges::any_of(actions, [this](const auto& action) { return !HasAction(action); });

    return !invalid;
}

void WebInterface::DrawGrid(const WorldGrid& grid, const std::vector<size_t>& itemIds,
                            const std::vector<size_t>& agentIds) {
    int canvasWidth;
    int canvasHeight;
    emscripten_get_canvas_element_size("#web-canvas", &canvasWidth, &canvasHeight);
    const double dpr = emscripten_get_device_pixel_ratio();
    
    canvasWidth = canvasWidth / dpr;
    canvasHeight = canvasHeight / dpr;
    
    const int totalPixelWidth = grid.GetWidth() * IMAGE_SIZE;
    const int totalPixelHeight = grid.GetHeight() * IMAGE_SIZE;
    
    const double widthRatio = static_cast<double>(canvasWidth) / totalPixelWidth;
    const double heightRatio = static_cast<double>(canvasHeight) / totalPixelHeight;
    
    const double desiredScale = widthRatio <= heightRatio ? widthRatio : heightRatio;
    const int drawSize = gsl::narrow_cast<int>(IMAGE_SIZE * desiredScale);
    
    const int leftOffset = (canvasWidth - drawSize * grid.GetWidth()) / 2;
    const int topOffset = (canvasHeight - drawSize * grid.GetHeight()) / 2;
    
    auto CellXToLeft = [&leftOffset, &drawSize](auto cellX) {
        return (leftOffset + drawSize * (cellX + 1)) - drawSize / 2;
    };
    auto CellYToTop = [&topOffset, &drawSize](auto cellY) { return topOffset + drawSize * (cellY + 1); };
    
    std::vector<std::string> symbolGrid(grid.GetHeight());
    
    // Load the world into the symbolGrid;
    for (size_t y = 0; y < grid.GetHeight(); ++y) {
        symbolGrid[y].resize(grid.GetWidth());
        for (size_t x = 0; x < grid.GetWidth(); ++x) {
            symbolGrid[y][x] = grid.GetSymbol(WorldPosition{x, y});
        }
    }
    
    mCanvas->Clear();
    
    int top = topOffset + drawSize;
    for (const auto& row: symbolGrid) {
        int left = leftOffset + drawSize / 2;
        for (char cell: row) {
            mCanvas->DrawTexture(mTextures.at(cell).as_handle(), left, top, desiredScale);
            left += drawSize;
        }
        top += drawSize;
    }
    
    // Substitute in items.
    for (size_t id: itemIds) {
        const ItemBase& item = world.GetItem(id);
        WorldPosition pos = item.GetLocation().AsWorldPosition();
        symbolGrid[pos.CellY()][pos.CellX()] = '+';
    }
    
    // Substitute in agents.
    for (const auto& agentId: agentIds) {
        const AgentBase& agent = world.GetAgent(agentId);
        WorldPosition pos = agent.GetLocation().AsWorldPosition();
        auto agentTexture = mTextures.at(agent.GetSymbol());
        auto agentLeft = CellXToLeft(pos.CellX());
        auto agentTop = CellYToTop(pos.CellY());
        mCanvas->DrawTexture(agentTexture.as_handle(), agentLeft, agentTop, desiredScale);
    }
    
    auto pos = GetLocation().AsWorldPosition();
    auto playerTexture = mTextures.at(GetSymbol());
    auto playerLeft = CellXToLeft(pos.CellX());
    auto playerTop = CellYToTop(pos.CellY());
    mCanvas->DrawTexture(playerTexture.as_handle(), playerLeft, playerTop, desiredScale);
}

void WebInterface::RenderFrame() {
    if (mPaused) {
        if (!mPauseMenu->IsVisible())
            mPauseMenu->ToggleVisibility();
    } else {
        if (mPauseMenu->IsVisible())
            mPauseMenu->ToggleVisibility();

        auto itemIds = world.GetKnownItems(*this);
        auto agentIds = world.GetKnownAgents(*this);
        auto grid = world.GetGrid();

        DrawGrid(grid, itemIds, agentIds);

        mPointsTextbox->SetText("Points: " + std::to_string(mPoints));
    }
    mRoot->Apply();
}

void WebInterface::HandlePause() {
    mPaused = !mPaused;
    RenderFrame();
}

#endif
