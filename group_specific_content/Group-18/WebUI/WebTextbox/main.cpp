#include "WebTextbox.hpp"
#include <iostream>
#include <emscripten.h>

// Global pointers so elements persist after main() ends
WebTextbox* t1 = nullptr;
WebTextbox* t2 = nullptr;
WebTextbox* t3 = nullptr;

int main() {
  // Basic HUD text
  t1 = new WebTextbox("Player HP: 100/100");
  t1->SetPosition(20, 70);
  t1->SetFontFamily("Arial");
  t1->SetFontSize(20);
  t1->SetColor("black");
  t1->SetBold(true);

  // A multi-line textbox 
  t2 = new WebTextbox("Battle Log:\n");
  t2->SetPosition(20, 120);
  t2->SetFontFamily("Times New Roman");
  t2->SetFontSize(16);
  t2->SetColor("red");
  t2->SetMaxWidth(300);

  // Append some test output
  t2->AppendText("You hit the Bat for 6 damage.\n");
  t2->AppendText("The Bat has fled the battle.\n");

  // A hidden debug textbox
  t3 = new WebTextbox("DEBUG: agent_test1 = 10, agent_test2 = 4");
  t3->SetPosition(20, 200);
  t3->SetColor("purple");
  t3->Hide();

  std::cout << "WebTextbox demo loaded" << std::endl;
  return 0;
}