#include "WebImage.hpp"
#include <iostream>
#include <emscripten.h>

using namespace cse498;

// Global pointers so images survive after main() ends
WebImage* img1 = nullptr;
WebImage* img2 = nullptr;
WebImage* img3 = nullptr;
WebImage* img4 = nullptr;

int main() {
  std::cout << "WebImage demo loading..." << std::endl;

  // Image 1: Basic image with Resize (maintain aspect ratio)
  img1 = new WebImage("https://via.placeholder.com/200x150", "Placeholder 200x150");
  img1->Resize(200, 150, true);  // Scale to fit, keep aspect ratio
  img1->SetOnLoadCallback([]() {
    std::cout << "Image 1 loaded successfully!" << std::endl;
  });
  img1->SetOnErrorCallback([]() {
    std::cout << "Image 1 failed to load (showing blank rect)" << std::endl;
  });

  // Image 2: SetSize (stretches, no aspect ratio preservation)
  img2 = new WebImage("https://via.placeholder.com/100x100", "Square Placeholder");
  img2->SetSize(200, 100);  // Stretch to exact 200x100
  img2->SetOpacity(0.6);    // 60% transparent
  img2->SetOnLoadCallback([]() {
    std::cout << "Image 2 loaded (60% opacity)!" << std::endl;
  });

  // Image 3: Invalid source - demonstrates error handling (BlankRect mode)
  img3 = new WebImage("invalid://not-a-real-image.png", "Missing Image");
  img3->Resize(150, 100);
  img3->SetErrorMode(ImageErrorMode::BlankRect);
  img3->SetPlaceholderColor("#FF6B6B");  // Red placeholder
  img3->SetOnErrorCallback([]() {
    std::cout << "Image 3 error handled: showing red placeholder" << std::endl;
  });

  // Image 4: Invalid source - demonstrates NoOp error mode
  img4 = new WebImage("also-invalid.png", "Another Missing");
  img4->Resize(150, 100);
  img4->SetErrorMode(ImageErrorMode::NoOp);
  img4->SetOnErrorCallback([]() {
    std::cout << "Image 4 error handled: no-op mode (broken icon)" << std::endl;
  });

  std::cout << "WebImage demo loaded" << std::endl;
  std::cout << "Image 1 ID: " << img1->Id() << " (resize with aspect ratio)" << std::endl;
  std::cout << "Image 2 ID: " << img2->Id() << " (stretched, 60% opacity)" << std::endl;
  std::cout << "Image 3 ID: " << img3->Id() << " (error -> blank rect)" << std::endl;
  std::cout << "Image 4 ID: " << img4->Id() << " (error -> no-op)" << std::endl;

  return 0;
}
