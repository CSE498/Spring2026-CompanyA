#include "WebButton.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <iostream>
#include <memory>

static constexpr int DEFAULT_SPACING = 10;
static constexpr int BUTTON_WIDTH = 150;
static constexpr int BUTTON_HEIGHT = 40;

int main() {
  auto layout = std::make_unique<WebLayout>("button-demo");
  layout->SetLayoutType(LayoutType::Vertical);
  layout->SetSpacing(DEFAULT_SPACING);

  auto btn1 = std::make_unique<WebButton>("Click Me");
  btn1->SetSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  btn1->SetBackgroundColor("#4CAF50");
  btn1->SetTextColor("white");
  btn1->SetCallback([]() {
    std::cout << "Button was clicked!" << std::endl;
  });
  btn1->MountToLayout(*layout);

  auto btn2 = std::make_unique<WebButton>("Disabled Button");
  btn2->SetSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  btn2->Disable();
  btn2->MountToLayout(*layout);

  auto btn3 = std::make_unique<WebButton>("Hidden Button");
  btn3->SetSize(BUTTON_WIDTH, BUTTON_HEIGHT);
  btn3->Hide();
  btn3->MountToLayout(*layout);

  layout->Apply();

  std::cout << "WebButton demo loaded" << std::endl;
  return 0;
}