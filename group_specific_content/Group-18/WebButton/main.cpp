#include "WebButton.hpp"
#include <iostream>
#include <emscripten.h>

// Global pointers so buttons survive after main() ends
WebButton* btn1 = nullptr;
WebButton* btn2 = nullptr;
WebButton* btn3 = nullptr;

int main() {
  btn1 = new WebButton("Click Me");
  btn1->SetSize(150, 40);
  btn1->SetPosition(20, 20);
  btn1->SetBackgroundColor("#4CAF50");
  btn1->SetTextColor("white");
  btn1->SetCallback([]() {
    std::cout << "Button was clicked!" << std::endl;
  });

  btn2 = new WebButton("Disabled Button");
  btn2->SetSize(150, 40);
  btn2->SetPosition(20, 80);
  btn2->Disable();

  btn3 = new WebButton("Hidden Button");
  btn3->SetSize(150, 40);
  btn3->SetPosition(20, 140);
  btn3->Hide();

  std::cout << "WebButton demo loaded" << std::endl;
  return 0;
}