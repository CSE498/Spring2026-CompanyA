#include "WebTextbox.hpp"
#include "WebLayout.hpp"
#include <iostream>
#include <emscripten.h>

// Globals so they persist
WebLayout* root = nullptr;
WebTextbox* hud = nullptr;
WebTextbox* battle_log = nullptr;
WebTextbox* debug = nullptr;

int player_hp = 100;

// Simulate HP ticking down
void Tick() 
{
  player_hp -= 1;
  if (player_hp < 0)
  {
    player_hp = 100;
  }

  hud->SetText("Player HP: " + std::to_string(player_hp) + "/100");

  if (player_hp % 10 == 0) 
  {
    battle_log->AppendText("Player took damage! HP now " + std::to_string(player_hp) + "\n");
  }

  // Toggle debug visibility occasionally
  if (player_hp % 25 == 0) 
  {
    debug->Show();
  } 
  else if (player_hp % 25 == 10) 
  {
    debug->Hide();
  }
}

int main() 
{
  // ----------------------------------------------------
  // Root layout (vertical stacking for demo)
  // ----------------------------------------------------
  root = new WebLayout();                 
  root->SetLayoutType(LayoutType::Vertical);
  root->SetSpacing(10);
  root->Apply();     
  // ----------------------------------------------------
  // HUD Text
  // ----------------------------------------------------
  hud = new WebTextbox("Player HP: 100/100");
  hud->SetFontFamily("Arial");
  hud->SetFontSize(22);
  hud->SetBold(true);
  hud->SetColor("#222222");

  hud->mountToLayout(*root, Alignment::Start);

  // ----------------------------------------------------
  // Battle Log
  // ----------------------------------------------------
  battle_log = new WebTextbox("Battle Log:\n");
  battle_log->SetFontFamily("Courier New");
  battle_log->SetFontSize(16);
  battle_log->SetColor("#AA0000");
  battle_log->SetMaxWidth(400);
  battle_log->SetWrap(true);
  battle_log->SetBackgroundColor("#FFFFFF"); // debug background

  battle_log->AppendText("You hit the Bat for 6 damage.\n");
  battle_log->AppendText("The Bat has fled the battle.\n");

  battle_log->mountToLayout(*root, Alignment::Start);

  // ----------------------------------------------------
  // Debug Text
  // ----------------------------------------------------
  debug = new WebTextbox("DEBUG: agent_test1 = 10, agent_test2 = 4");
  debug->SetFontFamily("monospace");
  debug->SetFontSize(14);
  debug->SetColor("purple");
  debug->SetBackgroundColor("#EEEEFF");
  debug->Hide();

  debug->mountToLayout(*root, Alignment::Start);

  // ----------------------------------------------------
  // Print bounding box demo
  // ----------------------------------------------------
  auto box = hud->GetBoundingBoxPx();
  std::cout << "HUD size: " << box.w << " x " << box.h << std::endl;

  // ----------------------------------------------------
  // Simulate game loop updates
  // ----------------------------------------------------
  emscripten_set_main_loop(Tick, 1, 1);

  std::cout << "WebTextbox enhanced demo loaded." << std::endl;
  return 0;
}