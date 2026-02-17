/**
 * @file TextTest.cpp
 * @author Kiana May
 * Disclaimer: Code clean up and comments utilized Claude Sonnet 4.5
 */

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../third-party/SDL2/include/SDL2/SDL.h"
#include "../../../third-party/SDL2/include/SDL2/SDL_image.h"
#include "../../../third-party/SDL2/include/SDL2/SDL_ttf.h"

#include "../../../source/Interfaces/GUI/Text.hpp"

using namespace cse498;

/**
 * SDL Mock for testing
 */
class SDLMock {
public:
 SDL_Window* window = nullptr;
 SDL_Renderer* renderer = nullptr;

 SDLMock() {
  REQUIRE(SDL_Init(SDL_INIT_VIDEO) == 0);

  window = SDL_CreateWindow("Test",
                            SDL_WINDOWPOS_UNDEFINED,
                            SDL_WINDOWPOS_UNDEFINED,
                            100, 100,
                            SDL_WINDOW_HIDDEN);
  REQUIRE(window != nullptr);

  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
  REQUIRE(renderer != nullptr);
 }

 ~SDLMock() {
  SDL_DestroyRenderer(renderer);
  SDL_DestroyWindow(window);
  SDL_Quit();
 }
};

TEST_CASE("Text Constructor", "[Text]")
{
 SDLMock mock;
 REQUIRE_NOTHROW(Text(mock.renderer));
}

TEST_CASE("Text Content", "[Text]")
{
 SDLMock mock;
 Text text(mock.renderer);

 text.SetContent("Hello World");
 CHECK(text.GetContent() == "Hello World");
}

TEST_CASE("Text Color", "[Text]")
{
 SDLMock mock;
 Text text(mock.renderer);

 text.SetColor(255, 100, 50);
 Color c = text.GetColor();
 CHECK(c.r == 255);
 CHECK(c.g == 100);
 CHECK(c.b == 50);
}

TEST_CASE("Text Size", "[Text]")
{
 SDLMock mock;
 Text text(mock.renderer);

 text.SetSize(24);
 CHECK(text.GetSize() == 24);
}

TEST_CASE("Empty content rendering", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    // Empty content should not throw when drawing
    text.SetContent("");
    REQUIRE_NOTHROW(text.Draw(0, 0));

    // Empty content should have zero dimensions
    CHECK(text.GetWidth() == 0);
    CHECK(text.GetHeight() == 0);
}

TEST_CASE("Reset style", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    // Change everything
    text.SetContent("Test");
    text.SetSize(32);
    text.SetColor(100, 100, 100);
    text.SetBold(true);
    text.SetItalic(true);

    // Reset
    text.ResetStyle();

    // Check defaults
    CHECK(text.GetSize() == 16);
    CHECK(text.IsBold() == false);
    CHECK(text.IsItalic() == false);

    Color c = text.GetColor();
    CHECK(c.r == 255);
    CHECK(c.g == 255);
    CHECK(c.b == 255);

    // Content should remain
    CHECK(text.GetContent() == "Test");
}

TEST_CASE("Color with alpha channel", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetColor(255, 100, 50, 128);
    Color c = text.GetColor();
    CHECK(c.r == 255);
    CHECK(c.g == 100);
    CHECK(c.b == 50);
    CHECK(c.a == 128);
}

TEST_CASE("Text dimensions change with size", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Test");
    text.SetSize(12);
    int height12 = text.GetHeight();

    text.SetSize(24);
    int height24 = text.GetHeight();

    // Larger font should be taller
    CHECK(height24 > height12);
}

TEST_CASE("Text dimensions change with content", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Short");
    int width1 = text.GetWidth();

    text.SetContent("Much longer text string");
    int width2 = text.GetWidth();

    // Longer text should be wider
    CHECK(width2 > width1);
}

TEST_CASE("Invalid font path fallback", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    // Try to load invalid font - should fall back to default
    REQUIRE_NOTHROW(text.SetFont("nonexistent/path/to/font.ttf"));

    // Should have fallen back to default font
    CHECK(text.GetFont() == "../source/Interfaces/GUI/fonts/OpenSans-Regular.ttf");
}

TEST_CASE("Renderer can be set after construction", "[Text]")
{
    Text text;  // No renderer
    CHECK(text.GetRenderer() == nullptr);

    SDLMock mock;
    text.SetRenderer(mock.renderer);
    CHECK(text.GetRenderer() == mock.renderer);
}

TEST_CASE("Unicode content", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    // Test with emoji and special characters
    text.SetContent("Hello 世界 🌍");
    CHECK(text.GetContent() == "Hello 世界 🌍");

    // Should have non-zero dimensions
    CHECK(text.GetWidth() > 0);
    CHECK(text.GetHeight() > 0);
}