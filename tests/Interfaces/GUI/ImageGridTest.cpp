/**
 * @file ImageGridTest.cpp
 * @author Deni Tepic
 */

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../third-party/SDL2/include/SDL2/SDL.h"
#include "../../../third-party/SDL2/include/SDL2/SDL_image.h"

#include "../../../source/Interfaces/GUI/ImageGrid.hpp"
#include "../../../source/Interfaces/GUI/ImageManager.hpp"

using namespace cse498;

/**
 * Helper class to initialize and clean up SDL
 * for testing rendering functionality.
 *
 * This mirrors the pattern used in ImageManagerTest.
 */
class SDLMock {
public:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDLMock() {
        // Initialize SDL video subsystem
        REQUIRE(SDL_Init(SDL_INIT_VIDEO) == 0);

        // Initialize PNG image support
        REQUIRE((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0);

        // Create a hidden window (no need to display)
        window = SDL_CreateWindow("Test",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  100, 100,
                                  SDL_WINDOW_HIDDEN);
        REQUIRE(window != nullptr);

        // Create software renderer for predictable behavior
        renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
        REQUIRE(renderer != nullptr);
    }

    ~SDLMock() {
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        IMG_Quit();
        SDL_Quit();
    }
};

// Test that constructor properly sets dimensions and tile size.
TEST_CASE("ImageGrid Constructor Initializes Correctly", "[ImageGrid]") {

    ImageGrid grid(3, 2, 32, 32);

    REQUIRE(grid.GetWidth() == 3);
    REQUIRE(grid.GetHeight() == 2);
    REQUIRE(grid.GetTileWidth() == 32);
    REQUIRE(grid.GetTileHeight() == 32);
}

// Verify that SetCell correctly stores image names and GetCell retrieves them.
TEST_CASE("ImageGrid SetCell and GetCell", "[ImageGrid]") {

    ImageGrid grid(2, 2, 32, 32);

    grid.SetCell(1, 1, "tree");

    REQUIRE(grid.GetCell(1, 1) == "tree");
}


// Ensure Clear removes all image names from grid.
TEST_CASE("ImageGrid Clear Removes All Images", "[ImageGrid]") {

    ImageGrid grid(2, 2, 32, 32);

    grid.SetCell(0, 0, "grass");
    grid.Clear();

    REQUIRE(grid.GetCell(0, 0).empty());
}


// Resize should preserve overlapping region and update grid dimensions correctly.
TEST_CASE("ImageGrid Resize Preserves Existing Cells", "[ImageGrid]") {

    ImageGrid grid(2, 2, 32, 32);

    grid.SetCell(1, 1, "tree");

    // Increase size
    grid.Resize(4, 4);

    REQUIRE(grid.GetWidth() == 4);
    REQUIRE(grid.GetHeight() == 4);

    // Original cell should still exist
    REQUIRE(grid.GetCell(1, 1) == "tree");
}



// Verify that Draw delegates to ImageManager without throwing exceptions.
TEST_CASE("ImageGrid Draw Integrates With ImageManager", "[ImageGrid]") {

    SDLMock mock;
    ImageManager manager(mock.renderer);

    // Load valid test image used in ImageManager tests
    std::string testImage = "Interfaces/GUI/images/ImageManagerTest.png";
    REQUIRE_NOTHROW(manager.load_image("test_img", testImage));

    ImageGrid grid(2, 2, 32, 32);

    // Assign image to one cell
    grid.SetCell(0, 0, "test_img");

    // Draw should not throw and should call draw_image internally
    REQUIRE_NOTHROW(grid.Draw(manager));
}
