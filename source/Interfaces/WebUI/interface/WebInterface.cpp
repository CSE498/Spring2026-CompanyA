#ifdef __EMSCRIPTEN__

#include "./WebInterface.hpp"
#include "../WebLayout/WebLayout.hpp"
#include "../WebCanvas/WebCanvas.hpp"
#include "../WebButton/WebButton.hpp"
#include "../WebTextbox/WebTextbox.hpp"
#include "../../../core/WorldBase.hpp"

#include <sstream>

using namespace cse498;

WebInterface::WebInterface(size_t id, const std::string & name, const WorldBase & world) : InterfaceBase(id, name, world) {
  // Create root layout hooking into "root" div
  mElements.emplace_back(std::make_unique<WebLayout>("root"));
  mRoot = static_cast<WebLayout*>(mElements.back().get());
  mRoot->SetLayoutType(LayoutType::None);

  // Create canvas
  mElements.emplace_back(std::make_unique<WebCanvas>("web-canvas"));
  mCanvas = static_cast<WebCanvas*>(mElements.back().get());
  mCanvas->SetBackgroundColor("#0c1017");
  mRoot->AddElement(mCanvas);

  // Create world textbox
  auto worldTextPtr = std::make_unique<WebTextbox>();
  mWorldTextbox = worldTextPtr.get();
  mWorldTextbox->SetCanvasPosition(300.0f, 150.0f);
  mWorldTextbox->SetFontSize(20);
  mWorldTextbox->SetColor("#ccc");
  mWorldTextbox->SetFontFamily("monospace");
  mCanvas->AddElement(std::move(worldTextPtr));

  // Create points textbox
  auto pointsTextPtr = std::make_unique<WebTextbox>();
  mPointsTextbox = pointsTextPtr.get();
  mPointsTextbox->SetCanvasPosition(10.0f, 50.0f);
  mPointsTextbox->SetFontSize(24);
  mPointsTextbox->SetColor("#ccc");
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
  titlePtr->SetColor("#ccc");
  titlePtr->MountToLayout(*mPauseMenu);

  // Create description textbox
  mElements.emplace_back(std::make_unique<WebTextbox>("press Escape to resume, or Q to quit."));
  WebTextbox* descPtr = static_cast<WebTextbox*>(mElements.back().get());
  descPtr->SetAlignment(WebTextbox::TextAlign::Center);
  descPtr->SetFontSize(24.0f);
  descPtr->SetColor("#ccc");
  descPtr->MountToLayout(*mPauseMenu);

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
      std::vector<std::string> symbol_grid(grid.GetHeight());

      // Load the world into the symbol_grid;
      for (size_t y=0; y < grid.GetHeight(); ++y) {
        symbol_grid[y].resize(grid.GetWidth());
        for (size_t x=0; x < grid.GetWidth(); ++x) {
          symbol_grid[y][x] = grid.GetSymbol(WorldPosition{x,y});
        }
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
        symbol_grid[pos.CellY()][pos.CellX()] = agent.GetSymbol();
      }

      auto pos = GetLocation().AsWorldPosition();
      symbol_grid[pos.CellY()][pos.CellX()] = GetSymbol();

      std::ostringstream out;

      // Print out the symbol_grid with a box around it.
      out << '+' << std::string(grid.GetWidth(),'-') << "+\n";
      for (const auto & row : symbol_grid) {
        out << "|";
        for (char cell : row) out << cell;
        out << "|\n";
      }
      out << '+' << std::string(grid.GetWidth(),'-') << "+\n";
      out << "\nW, A, S, D to move,";
      out << "\nEsc to pause,";
      out << "\nQ to quit,";
      mWorldDescription = out.str();
      out.flush();
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

    mWorldTextbox->SetText(mWorldDescription);
    mPointsTextbox->SetText("Points: " + std::to_string(mPoints));
  }
  mRoot->Apply();
}

void WebInterface::HandlePause() {
  mPaused = !mPaused;
  RenderFrame();
}

#endif