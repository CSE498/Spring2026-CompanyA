#include "WebImage.hpp"
#include <iostream>
#include <emscripten.h>

// Global pointers so images survive after main() ends
WebImage* img1 = nullptr;
WebImage* img2 = nullptr;
WebImage* img3 = nullptr;

int main() {
  std::cout << "WebImage demo loading..." << std::endl;

  // Create a basic image with a placeholder URL
  img1 = new WebImage("https://via.placeholder.com/200x150", "Placeholder Image 1");
  img1->SetSize(200, 150);
  img1->SetPosition(20, 20);
  img1->SetOnLoadCallback([]() {
    std::cout << "Image 1 loaded successfully!" << std::endl;
  });

  // Create a second image with different settings
  img2 = new WebImage("https://via.placeholder.com/100x100", "Square Placeholder");
  img2->SetSize(150, 100);
  img2->SetPosition(20, 200);
  img2->SetMaintainAspectRatio(false);  // Stretch to fit
  img2->SetOnLoadCallback([]() {
    std::cout << "Image 2 loaded successfully!" << std::endl;
  });

  // Create a hidden image
  img3 = new WebImage("https://via.placeholder.com/80x80", "Hidden Image");
  img3->SetSize(80, 80);
  img3->SetPosition(20, 330);
  img3->Hide();

  std::cout << "WebImage demo loaded" << std::endl;
  std::cout << "Image 1 ID: " << img1->Id() << std::endl;
  std::cout << "Image 2 ID: " << img2->Id() << std::endl;
  std::cout << "Image 3 ID: " << img3->Id() << " (hidden)" << std::endl;

  return 0;
}
