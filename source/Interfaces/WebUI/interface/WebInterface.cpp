#ifdef __EMSCRIPTEN__

#include "./WebInterface.hpp"
#include "../WebLayout/WebLayout.hpp"
#include "../WebCanvas/WebCanvas.hpp"
#include "../WebButton/WebButton.hpp"
#include "../WebTextbox/WebTextbox.hpp"
#include "../../../tools/Color.hpp"
#include "../../../core/WorldBase.hpp"
#include "../../../../third-party/gsl/gsl"

#include <array>
#include <sstream>
#include <algorithm>

using namespace cse498;

namespace {
constexpr cse498::Color kDimGray = cse498::Color::FromRGB255(0xcc, 0xcc, 0xcc);
constexpr cse498::Color kCanvasBg = cse498::Color::FromRGB255(0x0c, 0x10, 0x17);
static_assert(kDimGray.R() == 0xcc && kDimGray.G() == 0xcc && kDimGray.B() == 0xcc);
static_assert(kCanvasBg.R() == 0x0c && kCanvasBg.G() == 0x10 && kCanvasBg.B() == 0x17);
}

using emscripten::val;

static constexpr const char * PLAYER_IMAGE = "agents/playerCharacter/agent_player.png";
static constexpr const char * MONSTER_IMAGE = "agents/monsters/agent_monster_goblin.png";

static constexpr int IMAGE_SIZE = 256;

// Use EM_ASYNC_JS to load images synchronously with await
EM_ASYNC_JS(emscripten::EM_VAL, load_bitmap, (const char* path), {
    const response = await fetch("/assets/" + UTF8ToString(path));
    const blob = await response.blob();
    const bitmap = await createImageBitmap(blob);
    // We must return a handle that Emscripten can turn into a val
    return Emval.toHandle(bitmap);
});

// Wrapper to return a proper emscripten::val
val loadImage(const std::string& path) {
    auto handle = load_bitmap(path.c_str());
    return val::take_ownership(handle);
}

WebInterface::WebInterface(size_t id, const std::string & name, const WorldBase & world) : InterfaceBase(id, name, world) {
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

  std::ranges::for_each(cellTypes, [this](const CellType & cell){
    if (cell.name == "Unknown") return;
    mTextures.emplace(cell.symbol, loadImage(cell.desc));
  });

  mTextures.emplace('@', loadImage(PLAYER_IMAGE));
  mTextures.emplace('*', loadImage(MONSTER_IMAGE));

  RenderFrame();
}

size_t WebInterface::SelectAction(const WorldGrid & grid) {
  auto userAction = mInputManager.GetAction();

  if (mPaused && userAction != InputManager::ActiveAction::Pause && userAction != InputManager::ActiveAction::Quit) return 0;

  switch (userAction) {
    case InputManager::ActiveAction::Up:       return action_map["up"];
    case InputManager::ActiveAction::Left:     return action_map["left"];
    case InputManager::ActiveAction::Down:     return action_map["down"];
    case InputManager::ActiveAction::Right:    return action_map["right"];
    case InputManager::ActiveAction::Interact: ++mPoints; return action_map["interact"];
    case InputManager::ActiveAction::Pause:    return action_map["pause"];
    case InputManager::ActiveAction::Quit:     return action_map["quit"];
    case InputManager::ActiveAction::None:     return 0;
    default: return 0;
  }
}

void WebInterface::DrawGrid(const WorldGrid & grid,
                  const std::vector<size_t> & item_ids,
                  const std::vector<size_t> & agent_ids) {
    {
      int canvasWidth;
      int canvasHeight;
      emscripten_get_canvas_element_size("#web-canvas", &canvasWidth, &canvasHeight);
      double dpr = emscripten_get_device_pixel_ratio();

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

      auto CellXToLeft = [&leftOffset, &drawSize](auto cellX){ return (leftOffset + drawSize * (cellX + 1)) - drawSize / 2; };
      auto CellYToTop = [&topOffset, &drawSize](auto cellY){ return topOffset + drawSize * (cellY + 1); };

      std::vector<std::string> symbol_grid(grid.GetHeight());

      // Load the world into the symbol_grid;
      for (size_t y=0; y < grid.GetHeight(); ++y) {
        symbol_grid[y].resize(grid.GetWidth());
        for (size_t x=0; x < grid.GetWidth(); ++x) {
          symbol_grid[y][x] = grid.GetSymbol(WorldPosition{x,y});
        }
      }

      mCanvas->Clear();

      int top = topOffset + drawSize;
      for (const auto & row : symbol_grid) {
        int left = leftOffset + drawSize / 2;
        for (char cell : row) {
          mCanvas->DrawTexture(mTextures.at(cell).as_handle(), left, top, desiredScale);
          left += drawSize;
        }
        top += drawSize;
      }

      // Substitute in items.
      for (size_t id : item_ids) {
        const ItemBase & item = world.GetItem(id);
        WorldPosition pos = item.GetLocation().AsWorldPosition();
        symbol_grid[pos.CellY()][pos.CellX()] = '+';
      }

      // Substitute in agents.
      for (const auto & agent_id : agent_ids) {
        const AgentBase & agent = world.GetAgent(agent_id);
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
}

void WebInterface::RenderFrame() {
  if (mPaused) {
    if (!mPauseMenu->IsVisible()) mPauseMenu->ToggleVisibility();
  } else {
    if (mPauseMenu->IsVisible()) mPauseMenu->ToggleVisibility();

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
