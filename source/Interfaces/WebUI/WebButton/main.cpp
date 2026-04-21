/**
 * @file main.cpp
 * @brief Demo entry point for the WebButton WebUI component.
 *
 * Creates a vertical layout and three buttons (one active, one disabled,
 * one hidden) to demonstrate WebButton features in an Emscripten/WASM build.
 *
 */

#include <iostream>
#include <memory>
#include "../../../tools/Color.hpp"
#include "../WebLayout/WebLayout.hpp"
#include "WebButton.hpp"

using namespace cse498;

namespace {

static constexpr int DEFAULT_SPACING = 10;
static constexpr int BUTTON_WIDTH = 150;
static constexpr int BUTTON_HEIGHT = 40;

// Keep DOM-backed demo objects alive after main() returns under Emscripten.
std::unique_ptr<WebLayout> sLayout;
std::unique_ptr<WebButton> sBtn1;
std::unique_ptr<WebButton> sBtn2;
std::unique_ptr<WebButton> sBtn3;

} // namespace

/// @brief Entry point; constructs the button demo layout and mounts three buttons.
/// @return 0 on success.
int main() {
    sLayout = std::make_unique<WebLayout>("button-demo");
    sLayout->SetLayoutType(LayoutType::Vertical);
    sLayout->SetSpacing(DEFAULT_SPACING);

    sBtn1 = std::make_unique<WebButton>("Click Me");
    sBtn1->SetSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    sBtn1->SetBackgroundColor(Color::FromRGB255(76, 175, 80).ToHex());
    sBtn1->SetTextColor(Color::FromRGB255(255, 255, 255).ToHex());
    sBtn1->SetCallback([]() { std::cout << "Button was clicked!" << std::endl; });
    sBtn1->MountToLayout(*sLayout);

    sBtn2 = std::make_unique<WebButton>("Disabled Button");
    sBtn2->SetSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    sBtn2->Disable();
    sBtn2->MountToLayout(*sLayout);

    sBtn3 = std::make_unique<WebButton>("Hidden Button");
    sBtn3->SetSize(BUTTON_WIDTH, BUTTON_HEIGHT);
    sBtn3->Hide();
    sBtn3->MountToLayout(*sLayout);

    sLayout->Apply();

    std::cout << "WebButton demo loaded" << std::endl;
    return 0;
}
