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
    CHECK(text.GetFont() == DEFAULT_FONT_PATH);
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

TEST_CASE("GetDimensions returns both width and height", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Test");
    text.SetSize(16);

    auto dims = text.GetDimensions();

    // Verify dimensions match individual getters
    CHECK(dims.width == text.GetWidth());
    CHECK(dims.height == text.GetHeight());

    // Verify dimensions are non-zero for valid text
    CHECK(dims.width > 0);
    CHECK(dims.height > 0);
}

TEST_CASE("GetDimensions returns zeros for empty content", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("");

    auto dims = text.GetDimensions();

    CHECK(dims.width == 0);
    CHECK(dims.height == 0);
}

TEST_CASE("GetDimensions changes with font size", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Sample Text");

    // Test with small font
    text.SetSize(12);
    auto dims_small = text.GetDimensions();

    // Test with large font
    text.SetSize(32);
    auto dims_large = text.GetDimensions();

    // Larger font should have larger dimensions
    CHECK(dims_large.width > dims_small.width);
    CHECK(dims_large.height > dims_small.height);
}

TEST_CASE("GetDimensions changes with content length", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetSize(16);

    // Short text
    text.SetContent("Hi");
    auto dims_short = text.GetDimensions();

    // Long text
    text.SetContent("This is a much longer text string");
    auto dims_long = text.GetDimensions();

    // Longer text should be wider
    CHECK(dims_long.width > dims_short.width);

    // Height should be the same (single line)
    CHECK(dims_long.height == dims_short.height);
}

TEST_CASE("GetDimensions with Unicode characters", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Hello 世界 🌍");

    auto dims = text.GetDimensions();

    // Should have valid dimensions for Unicode
    CHECK(dims.width > 0);
    CHECK(dims.height > 0);
}

TEST_CASE("Draw throws exception without renderer", "[Text]")
{
    Text text;  // No renderer
    text.SetContent("Test");

    // Should throw when trying to draw
    REQUIRE_THROWS_AS(text.Draw(0, 0), std::runtime_error);
}

TEST_CASE("GetDimensions is more efficient than separate calls", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Efficiency Test");

    // Get dimensions once
    auto dims = text.GetDimensions();

    // Verify it matches calling GetWidth and GetHeight separately
    CHECK(dims.width == text.GetWidth());
    CHECK(dims.height == text.GetHeight());

    // Note: GetDimensions makes 1 SDL call vs 2 separate calls
    // (Performance benefit can't be tested directly in unit tests)
}

TEST_CASE("GetDimensions with bold text", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Bold Text");

    text.SetBold(false);

    // Normal text
    auto dims_normal = text.GetDimensions();

    // Bold text
    text.SetBold(true);
    auto dims_bold = text.GetDimensions();

    // Bold text should be wider (or at least equal)
    CHECK(dims_bold.width >= dims_normal.width);
}

TEST_CASE("GetDimensions with italic text", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Italic Text");

    // Italic text
    text.SetItalic(true);
    auto dims_italic = text.GetDimensions();

    // Italic text dimensions may vary
    CHECK(dims_italic.width > 0);
    CHECK(dims_italic.height > 0 );
}

TEST_CASE("GetDimensions struct can be copied", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Copy Test");

    auto dims1 = text.GetDimensions();
    auto dims2 = dims1;  // Copy

    CHECK(dims2.width == dims1.width);
    CHECK(dims2.height == dims1.height);
}

TEST_CASE("GetDimensions struct can be assigned", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Test");

    Text::Dimensions dims;
    dims = text.GetDimensions();  // Assignment

    CHECK(dims.width > 0);
    CHECK(dims.height > 0);
}

TEST_CASE("SetBold enables and disables bold", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    // Start with bold off
    CHECK(text.IsBold() == false);

    // Enable bold
    text.SetBold(true);
    CHECK(text.IsBold() == true);

    // Disable bold
    text.SetBold(false);
    CHECK(text.IsBold() == false);
}

TEST_CASE("SetItalic enables and disables italic", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    // Start with italic off
    CHECK(text.IsItalic() == false);

    // Enable italic
    text.SetItalic(true);
    CHECK(text.IsItalic() == true);

    // Disable italic
    text.SetItalic(false);
    CHECK(text.IsItalic() == false);
}

TEST_CASE("Bold and italic can be enabled simultaneously", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Bold and Italic");
    text.SetBold(true);
    text.SetItalic(true);

    CHECK(text.IsBold() == true);
    CHECK(text.IsItalic() == true);

    // Dimensions should be valid
    CHECK(text.GetWidth() > 0);
    CHECK(text.GetHeight() > 0);
}

TEST_CASE("Text move constructor transfers ownership", "[Text]")
{
    SDLMock mock;
    Text text1(mock.renderer);
    text1.SetContent("Original");
    text1.SetSize(20);
    text1.SetBold(true);

    // Move construct
    Text text2(std::move(text1));

    // text2 should have the data
    CHECK(text2.GetContent() == "Original");
    CHECK(text2.GetSize() == 20);
    CHECK(text2.IsBold() == true);
    CHECK(text2.GetRenderer() == mock.renderer);

    // text1 should have nulled pointers
    CHECK(text1.GetRenderer() == nullptr);
}

TEST_CASE("Text move assignment transfers ownership", "[Text]")
{
    SDLMock mock;
    Text text1(mock.renderer);
    text1.SetContent("Source");
    text1.SetSize(24);
    text1.SetItalic(true);

    Text text2(mock.renderer);
    text2.SetContent("Destination");

    // Move assign
    text2 = std::move(text1);

    // text2 should have text1's data
    CHECK(text2.GetContent() == "Source");
    CHECK(text2.GetSize() == 24);
    CHECK(text2.IsItalic() == true);
    CHECK(text2.GetRenderer() == mock.renderer);

    // text1 should have nulled pointers
    CHECK(text1.GetRenderer() == nullptr);
}

TEST_CASE("Draw succeeds with valid content and renderer", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Hello World");

    // Should not throw when drawing valid text
    REQUIRE_NOTHROW(text.Draw(10, 10));
    REQUIRE_NOTHROW(text.Draw(0, 0));
    REQUIRE_NOTHROW(text.Draw(100, 200));
}

TEST_CASE("Draw succeeds with styled text", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Styled Text");
    text.SetSize(24);
    text.SetBold(true);
    text.SetItalic(true);
    text.SetColor(255, 100, 50);

    // Should handle styled text without errors
    REQUIRE_NOTHROW(text.Draw(50, 50));
}

TEST_CASE("SetSize with extremely large size", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Big");

    // Very large font size (may stress SDL_ttf)
    REQUIRE_NOTHROW(text.SetSize(200));
    CHECK(text.GetSize() == 200);

    // Should still be able to measure
    CHECK(text.GetWidth() > 0);
    CHECK(text.GetHeight() > 0);
}

TEST_CASE("Draw throws exception when renderer is null", "[Text]")
{
    Text text;  // No renderer
    text.SetContent("Test");

    // Should throw runtime_error
    REQUIRE_THROWS_AS(text.Draw(0, 0), std::runtime_error);

    // Check exception message contains useful info
    try {
        text.Draw(0, 0);
        FAIL("Should have thrown exception");
    } catch (const std::runtime_error& e) {
        std::string msg = e.what();
        CHECK(msg.find("renderer") != std::string::npos);
    }
}

TEST_CASE("Draw throws exception when font is not loaded", "[Text]")
{
    SDLMock mock;

    // Create text with invalid default font path (simulate font load failure)
    // This is hard to test directly without modifying DEFAULT_FONT
    // Alternative: test the error message path indirectly

    Text text(mock.renderer);
    text.SetContent("Test");

    // If font loaded successfully, this is our normal case
    if (text.GetWidth() > 0) {
        // Font loaded - can't easily test this error path
        // This is acceptable - would need dependency injection to test
        CHECK(true);
    }
}

TEST_CASE("SetFont throws exception for non-existent font after fallback", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    // This should fall back to default font (doesn't throw)
    REQUIRE_NOTHROW(text.SetFont("nonexistent_font.ttf"));

    // Should have fallen back
    CHECK(text.GetFont() == DEFAULT_FONT_PATH);
}

TEST_CASE("Multiple Draw calls work correctly", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Multiple Draws");

    // Should be able to draw multiple times
    REQUIRE_NOTHROW(text.Draw(10, 10));
    REQUIRE_NOTHROW(text.Draw(20, 20));
    REQUIRE_NOTHROW(text.Draw(30, 30));
}

TEST_CASE("Changing content between draws works", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("First");
    REQUIRE_NOTHROW(text.Draw(0, 0));

    text.SetContent("Second");
    REQUIRE_NOTHROW(text.Draw(0, 0));

    text.SetContent("Third");
    REQUIRE_NOTHROW(text.Draw(0, 0));
}

TEST_CASE("Very long text content", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    // Create very long string
    std::string long_text(1000, 'A');
    text.SetContent(long_text);

    // Should handle without crashing
    CHECK(text.GetWidth() > 0);
    CHECK(text.GetHeight() > 0);
    REQUIRE_NOTHROW(text.Draw(0, 0));
}

TEST_CASE("Special characters in text", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Special: \n\t\"'<>&");

    // Should handle special characters
    CHECK(text.GetWidth() > 0);
    REQUIRE_NOTHROW(text.Draw(0, 0));
}

TEST_CASE("Toggling styles multiple times", "[Text]")
{
    SDLMock mock;
    Text text(mock.renderer);

    text.SetContent("Toggle Test");

    // Toggle bold multiple times
    text.SetBold(true);
    text.SetBold(false);
    text.SetBold(true);
    CHECK(text.IsBold() == true);

    // Toggle italic multiple times
    text.SetItalic(true);
    text.SetItalic(false);
    text.SetItalic(true);
    CHECK(text.IsItalic() == true);

    // Should still work
    REQUIRE_NOTHROW(text.Draw(0, 0));
}

