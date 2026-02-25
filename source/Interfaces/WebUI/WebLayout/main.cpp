// Demo: Game UI styled layouts with WebImage elements
// Compile from this directory (`group_specific_content/Group-18/WebLayout`):
// em++ -std=c++23 -O2 \
//   -s WASM=1 \
//   -s EXPORTED_FUNCTIONS="['_main','_WebImage_handleLoad']" \
//   -s EXPORTED_RUNTIME_METHODS="['ccall','cwrap']" \
//   --bind \
//   main.cpp WebLayout.cpp ../WebImage/WebImage.cpp ../WebButton/WebButton.cpp \
//   -I. -I.. \
//   -o weblayout_demo.js

#include "WebLayout.hpp"
#include <iostream>
#include <vector>
#include "../WebImage/WebImage.hpp"
#include "../WebButton/WebButton.hpp"

using std::cout, std::endl, std::string, std::vector, std::to_string;

// Global containers (game UI panels)
static WebLayout *root = nullptr;
static WebLayout *statusPanel = nullptr;
static WebButton *startButton = nullptr;
static WebLayout *inventoryPanel = nullptr;

static WebButton *toggleStatusButton = nullptr;

static vector<WebImage *> images;

static WebImage *MakeImage(const string &url, int w, int h, const string &alt = "") {
  WebImage *img = new WebImage(url, alt);
  img->SetSize(w, h);
  images.push_back(img);
  return img;
}

int main() {
  cout << "Game UI Demo starting..." << endl;

  // Root container with dark background
  root = new WebLayout("app-root");
  root->SetLayoutType(LayoutType::Vertical);
  root->SetSpacing(16);
  root->SetPadding(20);
  root->SetBackgroundColor("#1a1a1a");

  // =====================
  // MAIN MENU (Dark panel with glowing border)
  // =====================
  WebLayout* mainMenu = new WebLayout();
  mainMenu->SetLayoutType(LayoutType::Vertical);
  mainMenu->SetJustification(Justification::Center);
  mainMenu->SetAlignItems(Alignment::Center);
  mainMenu->SetSpacing(10);
  mainMenu->SetPadding(20);
  mainMenu->SetMargin(10);
  mainMenu->SetWidth(300);
  mainMenu->SetBackgroundColor("#2a2a3e");
  mainMenu->SetBorderColor("#00ff88");
  mainMenu->SetBorderWidth(2);
  mainMenu->SetBorderRadius(8);
  mainMenu->SetBoxShadow("0 0 20px rgba(0, 255, 136, 0.5)");

  toggleStatusButton = new WebButton("Toggle Main Menu");
  toggleStatusButton->SetCallback([mainMenu]() {
    mainMenu->ToggleVisibility();
    mainMenu->Apply();
  });
  toggleStatusButton->SetSize(100, 70);
  toggleStatusButton->SetBackgroundColor("#00ff88");
  toggleStatusButton->SetTextColor("#1a1a1a");
  toggleStatusButton->MountToLayout(*root, Alignment::Center);

  mainMenu->MountToLayout(*root, Alignment::Start);

  // Menu items (4 game menu options shown as icons)
  for (int i = 1; i <= 4; ++i) {
    string url = "https://placehold.co/260x50?text=Menu+" + to_string(i);
    WebImage *menuItem = MakeImage(url, 260, 50, "Menu Item " + to_string(i));
    menuItem->MountToLayout(*mainMenu, Alignment::Stretch);
  }

  // =====================
  // STATUS PANEL (Player stats area with orange/red theme)
  // =====================
  statusPanel = new WebLayout();
  statusPanel->SetLayoutType(LayoutType::Horizontal);
  statusPanel->SetJustification(Justification::SpaceAround);
  statusPanel->SetAlignItems(Alignment::Center);
  statusPanel->SetSpacing(12);
  statusPanel->SetPadding(15);
  statusPanel->SetHeight(100);
  statusPanel->SetBackgroundColor("#3d2817");
  statusPanel->SetBorderColor("#ff6b1a");
  statusPanel->SetBorderWidth(2);
  statusPanel->SetBorderRadius(6);
  statusPanel->SetBoxShadow("0 4px 12px rgba(255, 107, 26, 0.3)");
  statusPanel->MountToLayout(*root, Alignment::Stretch);

  // Status items (HP, Mana, XP as small icons)
  for (int i = 0; i < 3; ++i) {
    WebImage *stat = MakeImage("https://placehold.co/60x70?text=Stat", 60, 70, "Stat " + to_string(i));
    stat->MountToLayout(*statusPanel, Alignment::Center);
  }

  // =====================
  // INVENTORY PANEL (Grid-based inventory with metallic blue theme)
  // =====================
  inventoryPanel = new WebLayout();
  inventoryPanel->SetLayoutType(LayoutType::Grid);
  inventoryPanel->SetJustification(Justification::Center);
  inventoryPanel->SetAlignItems(Alignment::Center);
  inventoryPanel->SetSpacing(8);
  inventoryPanel->SetPadding(16);
  inventoryPanel->SetWidth(700);
  inventoryPanel->SetHeight(700);
  inventoryPanel->SetBackgroundColor("#1a2a3a");
  inventoryPanel->SetBorderColor("#4a90e2");
  inventoryPanel->SetBorderWidth(3);
  inventoryPanel->SetBorderRadius(10);
  inventoryPanel->SetBoxShadow("inset 0 0 10px rgba(74, 144, 226, 0.3), 0 8px 16px rgba(0, 0, 0, 0.5)");
  inventoryPanel->MountToLayout(*root, Alignment::Stretch);

  // 12 inventory slots in a 4x3 grid
  for (int i = 1; i <= 12; ++i) {
    string url = "https://placehold.co/100x100?text=Item+" + to_string(i);
    WebImage *item = MakeImage(url, 100, 100, "Inventory Item " + to_string(i));
    int idx = i - 1;
    int r = idx / 4;  // 0-based row
    int c = idx % 4;  // 0-based col
    item->SetGridPosition(r, c);
    item->MountToLayout(*inventoryPanel, Alignment::Center);
  }

  // =====================
  // EQUIPMENT PANEL (Vertical layout with purple theme for equipped items)
  // =====================
  WebLayout *equipmentPanel = new WebLayout();
  equipmentPanel->SetLayoutType(LayoutType::Vertical);
  equipmentPanel->SetJustification(Justification::Center);
  equipmentPanel->SetAlignItems(Alignment::Center);
  equipmentPanel->SetSpacing(8);
  equipmentPanel->SetPadding(12);
  equipmentPanel->SetWidth(250);
  equipmentPanel->SetBackgroundColor("#2d1b3d");
  equipmentPanel->SetBorderColor("#c055f0");
  equipmentPanel->SetBorderWidth(2);
  equipmentPanel->SetBorderRadius(8);
  equipmentPanel->SetOpacity(0.95);
  equipmentPanel->SetBoxShadow("0 0 15px rgba(192, 85, 240, 0.4)");
  equipmentPanel->MountToLayout(*root, Alignment::Start);

  // Equipment slots
  for (int i = 0; i < 5; ++i) {
    string url = "https://placehold.co/220x40?text=Equipped";
    WebImage *equip = MakeImage(url, 220, 40, "Equipment " + to_string(i));
    equip->MountToLayout(*equipmentPanel, Alignment::Stretch);
  }

  // Apply all layouts
  mainMenu->Apply();
  statusPanel->Apply();
  inventoryPanel->Apply();
  equipmentPanel->Apply();
  root->Apply();

  cout << "Game UI demo constructed successfully!" << endl;
  cout << "Root ID: " << root->Id() << endl;
  cout << "Main Menu ID: " << mainMenu->Id() << endl;
  cout << "Status Panel ID: " << statusPanel->Id() << endl;
  cout << "Inventory Panel ID: " << inventoryPanel->Id() << endl;
  cout << "Equipment Panel ID: " << equipmentPanel->Id() << endl;
  cout << "\nTotal images created: " << images.size() << endl;
  cout << "Game UI demo ready!" << endl;

  return 0;
}
