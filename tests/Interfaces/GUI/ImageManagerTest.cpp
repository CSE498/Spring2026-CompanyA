/**
 * @file ImageManagerTest.cpp
 * @author Sitara Baxendale
 *
 * AI Dislaimer: Used ChatGPT to create the SDLMock class for
 * creating & destroying a test window and renderer.
 */

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../third-party/SDL2/include/SDL2/SDL.h"
#include "../../../third-party/SDL2/include/SDL2/SDL_image.h"

#include "../../../source/Interfaces/GUI/ImageManager.hpp"

namespace cse498 {

/**
 * Class for initalizing and shutting down the SDL
 */
class SDLMock {
public:
  // sdl window & renderer pointers
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;

  SDLMock() {
    // initialize sdl video subsystem & image
    REQUIRE(SDL_Init(SDL_INIT_VIDEO) == 0);
    REQUIRE((IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG) != 0);

    // create a window for rendering
    window =
        SDL_CreateWindow("Test", SDL_WINDOWPOS_UNDEFINED,
                         SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_HIDDEN);
    REQUIRE(window != nullptr);

    // create a renderer
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    REQUIRE(renderer != nullptr);
  }

  ~SDLMock() {
    // clean up the mock resources
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
  }
};

TEST_CASE("Constructor", "[ImageManager]") {
  // set up SDL
  SDLMock mock;

  // ensures constructor works with valid renderer
  REQUIRE_NOTHROW(ImageManager(mock.renderer));
}

TEST_CASE("Load and get images", "[ImageManager]") {
  // set up SDL
  SDLMock mock;
  ImageManager manager(mock.renderer);

  std::string testImage = std::string(TEST_IMAGE_DIR) + "/ImageManagerTest.png";

  // load image & confirm existence
  REQUIRE_NOTHROW(manager.load_image("test_img", testImage));
  CHECK(manager.has_image("test_img") == true);
  CHECK(manager.get_texture("test_img") != nullptr);
}

TEST_CASE("Duplicate images", "[ImageManager]") {
  // set up SDL
  SDLMock mock;
  ImageManager manager(mock.renderer);

  // load image
  std::string testImage = std::string(TEST_IMAGE_DIR) + "/ImageManagerTest.png";
  manager.load_image("test_img", testImage);

  // check for error when trying to load same image again
  REQUIRE_THROWS_AS(manager.load_image("test_img", testImage),
                    std::runtime_error);
}

TEST_CASE("Loading invalid file", "[ImageManager]") {
  // set up SDL
  SDLMock mock;
  ImageManager manager(mock.renderer);

  // check for error when trying to load a non-existence image file
  REQUIRE_THROWS_AS(manager.load_image("invalid", "invalid.png"),
                    std::runtime_error);
}

TEST_CASE("Invalid file in draw_image", "[ImageManager]") {
  // set up SDL
  SDLMock mock;
  ImageManager manager(mock.renderer);

  // confirm it returns false
  CHECK(manager.draw_image("invalid", 0, 0) == false);
}

TEST_CASE("Valid file in draw_image", "[ImageManager]") {
  // set up SDL
  SDLMock mock;
  ImageManager manager(mock.renderer);

  // load image
  std::string testImage = std::string(TEST_IMAGE_DIR) + "/ImageManagerTest.png";
  manager.load_image("test_img", testImage);

  // confirm it returns true
  CHECK(manager.draw_image("test_img", 10, 10) == true);
}

TEST_CASE("Loading empty strings", "[ImageManager]") {
  // set up SDL
  SDLMock mock;
  ImageManager manager(mock.renderer);

  std::string testImage = std::string(TEST_IMAGE_DIR) + "/ImageManagerTest.png";

  // check for invalid_argument on empty name
  REQUIRE_THROWS_AS(manager.load_image("", testImage), std::invalid_argument);

  // check for invalid_argument on empty path
  REQUIRE_THROWS_AS(manager.load_image("test_img", ""), std::invalid_argument);
}

TEST_CASE("Extreme coordinates in draw_image", "[ImageManager]") {
  // set up SDL
  SDLMock mock;
  ImageManager manager(mock.renderer);

  // load image
  std::string testImage = std::string(TEST_IMAGE_DIR) + "/ImageManagerTest.png";
  manager.load_image("test_img", testImage);

  // Smoke tests for extreme coordinates to ensure SDL doesn't crash
  CHECK(manager.draw_image("test_img", -9999, -9999) == true);
  CHECK(manager.draw_image("test_img", 99999, 99999) == true);
}

} // namespace cse498