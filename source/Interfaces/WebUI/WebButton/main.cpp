#include "WebButton.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <iostream>
#include <emscripten.h>

WebLayout* layout = nullptr;
WebButton* btn1 = nullptr;
WebButton* btn2 = nullptr;
WebButton* btn3 = nullptr;

int main() {
  layout = new WebLayout("button-demo");
  layout->SetLayoutType(LayoutType::Vertical);
  layout->SetSpacing(10);

  btn1 = new WebButton("Click Me");
  btn1->SetSize(150, 40);
  btn1->SetBackgroundColor("#4CAF50");
  btn1->SetTextColor("white");
  btn1->SetCallback([]() {
    std::cout << "Button was clicked!" << std::endl;
  });
  btn1->MountToLayout(*layout);

  btn2 = new WebButton("Disabled Button");
  btn2->SetSize(150, 40);
  btn2->Disable();
  btn2->MountToLayout(*layout);

  btn3 = new WebButton("Hidden Button");
  btn3->SetSize(150, 40);
  btn3->Hide();
  btn3->MountToLayout(*layout);

  layout->Apply();

  std::cout << "WebButton demo loaded" << std::endl;
  return 0;
}