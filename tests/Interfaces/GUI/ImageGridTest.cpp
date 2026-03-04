/**
 * @file ImageGridTest.cpp
 * @author Deni Tepic
 */

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../third-party/SDL2/include/SDL2/SDL.h"
#include "../../../third-party/SDL2/include/SDL2/SDL_image.h"

#include "../../../source/Interfaces/GUI/ImageGrid.hpp"
#include "../../../source/Interfaces/GUI/ImageManager.hpp"

namespace cse498 {

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
        REQUIRE(SDL_Init(SDL_INIT_VIDEO) == 0);
        REQUIRE((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0);

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

// Constructor should throw if tile dimensions are zero.
TEST_CASE("ImageGrid Constructor Rejects Zero Tile Size", "[ImageGrid]") {

    REQUIRE_THROWS_AS(
        ImageGrid(2, 2, 0, 32),
        std::invalid_argument
    );

    REQUIRE_THROWS_AS(
        ImageGrid(2, 2, 32, 0),
        std::invalid_argument
    );
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
    REQUIRE(grid.GetCell(1, 1) == "tree");
}

// Resize smaller should drop cells outside new bounds.
TEST_CASE("ImageGrid Resize Smaller Drops Out Of Bounds Cells", "[ImageGrid]") {

    ImageGrid grid(3, 3, 32, 32);

    grid.SetCell(2, 2, "tree");

    grid.Resize(1, 1);

    REQUIRE(grid.GetWidth() == 1);
    REQUIRE(grid.GetHeight() == 1);
}

// Verify that Draw delegates to ImageManager without throwing exceptions.
TEST_CASE("ImageGrid Draw Integrates With ImageManager", "[ImageGrid]") {

    SDLMock mock;
    ImageManager manager(mock.renderer);

    std::string testImage = "Interfaces/GUI/images/ImageManagerTest.png";
    REQUIRE_NOTHROW(manager.load_image("test_img", testImage));

    ImageGrid grid(2, 2, 32, 32);
    grid.SetCell(0, 0, "test_img");

    REQUIRE_NOTHROW(grid.Draw(manager));
}

// Drawing an empty grid should not throw.
TEST_CASE("ImageGrid Draw Empty Grid", "[ImageGrid]") {

    SDLMock mock;
    ImageManager manager(mock.renderer);

    ImageGrid grid(2, 2, 32, 32);

    REQUIRE_NOTHROW(grid.Draw(manager));
}

} // namespace cse498