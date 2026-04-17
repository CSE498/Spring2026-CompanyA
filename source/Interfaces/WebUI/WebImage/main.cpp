/**
 * @file main.cpp
 * @brief Demo entry point for the WebImage WebUI component.
 *
 * Demonstrates four WebImage usage scenarios:
 *  -# Resize with aspect ratio preservation.
 *  -# SetSize (stretch, no aspect ratio) with reduced opacity.
 *  -# Invalid source with BlankRect error mode (red placeholder).
 *  -# Invalid source with NoOp error mode (broken icon).
 *
 */

#include <iostream>
#include <memory>
#include "../../../tools/Color.hpp"
#include "../WebLayout/WebLayout.hpp"
#include "WebImage.hpp"

using namespace cse498;

namespace {

constexpr int kDefaultSpacing = 12;

// Keep DOM-backed demo objects alive after main() returns under Emscripten.
std::unique_ptr<WebLayout> sLayout;
std::unique_ptr<WebImage> sImg1;
std::unique_ptr<WebImage> sImg2;
std::unique_ptr<WebImage> sImg3;
std::unique_ptr<WebImage> sImg4;

} // namespace

/// @brief Entry point; creates four WebImage instances to exercise the WebImage API.
/// @return 0 on success.
int main() {
    std::cout << "WebImage demo loading..." << std::endl;

    sLayout = std::make_unique<WebLayout>("webimage-demo");
    sLayout->SetLayoutType(LayoutType::Vertical);
    sLayout->SetSpacing(kDefaultSpacing);

    // Image 1: Basic image with Resize (maintain aspect ratio)
    sImg1 = std::make_unique<WebImage>("https://via.placeholder.com/200x150", "Placeholder 200x150");
    sImg1->Resize(200, 150, true); // Scale to fit, keep aspect ratio
    sImg1->SetOnLoadCallback([]() { std::cout << "Image 1 loaded successfully!" << std::endl; });
    sImg1->SetOnErrorCallback([]() { std::cout << "Image 1 failed to load (showing blank rect)" << std::endl; });
    sImg1->MountToLayout(*sLayout, Alignment::Start);

    // Image 2: SetSize (stretches, no aspect ratio preservation)
    sImg2 = std::make_unique<WebImage>("https://via.placeholder.com/100x100", "Square Placeholder");
    sImg2->SetSize(200, 100); // Stretch to exact 200x100
    sImg2->SetOpacity(0.6); // 60% transparent
    sImg2->SetOnLoadCallback([]() { std::cout << "Image 2 loaded (60% opacity)!" << std::endl; });
    sImg2->MountToLayout(*sLayout, Alignment::Start);

    // Image 3: Invalid source - demonstrates error handling (BlankRect mode)
    sImg3 = std::make_unique<WebImage>("invalid://not-a-real-image.png", "Missing Image");
    sImg3->Resize(150, 100);
    sImg3->SetErrorMode(ImageErrorMode::BlankRect);
    sImg3->SetPlaceholderColor(Color::FromRGB255(255, 107, 107).ToHex()); // Red placeholder
    sImg3->SetOnErrorCallback([]() { std::cout << "Image 3 error handled: showing red placeholder" << std::endl; });
    sImg3->MountToLayout(*sLayout, Alignment::Start);

    // Image 4: Invalid source - demonstrates NoOp error mode
    sImg4 = std::make_unique<WebImage>("also-invalid.png", "Another Missing");
    sImg4->Resize(150, 100);
    sImg4->SetErrorMode(ImageErrorMode::NoOp);
    sImg4->SetOnErrorCallback([]() { std::cout << "Image 4 error handled: no-op mode (broken icon)" << std::endl; });
    sImg4->MountToLayout(*sLayout, Alignment::Start);

    sLayout->Apply();

    std::cout << "WebImage demo loaded" << std::endl;
    std::cout << "Image 1 ID: " << sImg1->Id() << " (resize with aspect ratio)" << std::endl;
    std::cout << "Image 2 ID: " << sImg2->Id() << " (stretched, 60% opacity)" << std::endl;
    std::cout << "Image 3 ID: " << sImg3->Id() << " (error -> blank rect)" << std::endl;
    std::cout << "Image 4 ID: " << sImg4->Id() << " (error -> no-op)" << std::endl;

    return 0;
}
