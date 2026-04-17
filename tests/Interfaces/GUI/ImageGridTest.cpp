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
 */
class SDLMock {
public:
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;

    SDLMock() {
        REQUIRE(SDL_Init(SDL_INIT_VIDEO) == 0);
        REQUIRE((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0);

        window =
                SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_HIDDEN);
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

/**
 * Test that constructor properly sets dimensions and tile size.
 */
TEST_CASE("ImageGrid Constructor Initializes Correctly", "[ImageGrid]") {

    ImageGrid grid(3, 2, 32, 32);

    REQUIRE(grid.GetWidth() == 3);
    REQUIRE(grid.GetHeight() == 2);
    REQUIRE(grid.GetTileWidth() == 32);
    REQUIRE(grid.GetTileHeight() == 32);
}

/**
 * Verify that SetCell correctly stores image names and GetCell retrieves them.
 */
TEST_CASE("ImageGrid SetCell and GetCell", "[ImageGrid]") {

    ImageGrid grid(2, 2, 32, 32);

    grid.SetCell(1, 1, "tree");

    REQUIRE(grid.GetCell(1, 1) == "tree");
}

/**
 * Ensure Clear removes all image names from the grid.
 */
TEST_CASE("ImageGrid Clear Removes All Images", "[ImageGrid]") {

    ImageGrid grid(2, 2, 32, 32);
    grid.SetCell(0, 0, "grass");
    grid.SetCell(1, 1, "tree");

    grid.Clear();

    REQUIRE(grid.GetCell(0, 0).empty());
    REQUIRE(grid.GetCell(1, 1).empty());
}

/**
 * Ensure Fill assigns the same image name to every cell.
 */
TEST_CASE("ImageGrid Fill Sets Every Cell", "[ImageGrid]") {

    ImageGrid grid(3, 2, 32, 32);

    grid.Fill("grass");

    for (size_t y = 0; y < grid.GetHeight(); ++y) {
        for (size_t x = 0; x < grid.GetWidth(); ++x) {
            REQUIRE(grid.GetCell(x, y) == "grass");
        }
    }
}

/**
 * Ensure IsValid returns true for in-bounds coordinates
 * and false for out-of-bounds coordinates.
 */
TEST_CASE("ImageGrid IsValid Recognizes Valid Coordinates", "[ImageGrid]") {

    ImageGrid grid(3, 2, 32, 32);

    REQUIRE(grid.IsValid(0, 0));
    REQUIRE(grid.IsValid(2, 1));

    REQUIRE_FALSE(grid.IsValid(3, 0));
    REQUIRE_FALSE(grid.IsValid(0, 2));
    REQUIRE_FALSE(grid.IsValid(10, 10));
}

/**
 * Resize should preserve overlapping region and update grid dimensions correctly.
 */
TEST_CASE("ImageGrid Resize Preserves Existing Cells", "[ImageGrid]") {

    ImageGrid grid(2, 2, 32, 32);
    grid.SetCell(1, 1, "tree");

    grid.Resize(4, 4);

    REQUIRE(grid.GetWidth() == 4);
    REQUIRE(grid.GetHeight() == 4);
    REQUIRE(grid.GetCell(1, 1) == "tree");
}

/**
 * Resize smaller should drop cells outside new bounds.
 */
TEST_CASE("ImageGrid Resize Smaller Drops Out Of Bounds Cells", "[ImageGrid]") {

    ImageGrid grid(3, 3, 32, 32);
    grid.SetCell(0, 0, "grass");
    grid.SetCell(2, 2, "tree");

    grid.Resize(1, 1);

    REQUIRE(grid.GetWidth() == 1);
    REQUIRE(grid.GetHeight() == 1);
    REQUIRE(grid.GetCell(0, 0) == "grass");
}

/**
 * Verify that Draw delegates to ImageManager without throwing exceptions.
 */
TEST_CASE("ImageGrid Draw Integrates With ImageManager", "[ImageGrid]") {

    SDLMock mock;
    ImageManager manager(mock.renderer);

    std::string testImage = std::string(TEST_IMAGE_DIR) + "/ImageManagerTest.png";
    auto result = manager.LoadImage("test_img", testImage);
    REQUIRE(result);

    ImageGrid grid(2, 2, 32, 32);
    grid.SetCell(0, 0, "test_img");

    REQUIRE_NOTHROW(grid.Draw(manager));
}

/**
 * Drawing an empty grid should not throw.
 */
TEST_CASE("ImageGrid Draw Empty Grid", "[ImageGrid]") {

    SDLMock mock;
    ImageManager manager(mock.renderer);

    ImageGrid grid(2, 2, 32, 32);

    REQUIRE_NOTHROW(grid.Draw(manager));
}

/**
 * Verify that DrawViewport renders a visible portion of the grid
 * without throwing exceptions.
 */
TEST_CASE("ImageGrid DrawViewport Integrates With ImageManager", "[ImageGrid]") {

    SDLMock mock;
    ImageManager manager(mock.renderer);

    std::string testImage = std::string(TEST_IMAGE_DIR) + "/ImageManagerTest.png";
    auto result = manager.LoadImage("test_img", testImage);
    REQUIRE(result);

    ImageGrid grid(4, 4, 32, 32);
    grid.SetCell(1, 1, "test_img");
    grid.SetCell(2, 2, "test_img");

    REQUIRE_NOTHROW(grid.DrawViewport(manager, 0, 0, 64, 64));
}

/**
 * Drawing an empty viewport region should not throw.
 */
TEST_CASE("ImageGrid DrawViewport Empty Grid", "[ImageGrid]") {

    SDLMock mock;
    ImageManager manager(mock.renderer);

    ImageGrid grid(4, 4, 32, 32);

    REQUIRE_NOTHROW(grid.DrawViewport(manager, 0, 0, 64, 64));
}

} // namespace cse498
