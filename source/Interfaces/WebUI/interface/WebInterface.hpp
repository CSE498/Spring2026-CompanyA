#pragma once

#include "../../../core/InterfaceBase.hpp"
#include "../InputManager.hpp"
#include "../internal/IDomElement.hpp"


#include <string>
#include <vector>
#include <memory>

namespace cse498 {

class WebLayout;
class WebTextbox;
class WebCanvas;

class WebInterface : public InterfaceBase {
  
public:

  WebInterface(size_t id, const std::string & name, const WorldBase & world);
  ~WebInterface() = default;

  [[nodiscard]] size_t SelectAction(const WorldGrid & grid) override;

  void RenderFrame() override;

  void HandlePause();

private:
  InputManager mInputManager{*this};
  std::vector<std::unique_ptr<IDomElement>> mElements{};
  int mPoints{0};
  std::string mWorldDescription{};
  WebLayout * mRoot;
  WebLayout * mPauseMenu;
  WebCanvas * mCanvas;
  WebTextbox * mWorldTextbox;
  WebTextbox * mPointsTextbox;


  void DrawGrid(const WorldGrid & grid,
                  const std::vector<size_t> & itemIds,
                  const std::vector<size_t> & agentIds);

};

}