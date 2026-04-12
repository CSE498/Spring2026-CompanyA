/**
 * @file MenuTest.cpp
 * @author Anagha Jammalamadaka
 *
 * AI Disclaimer:
 *  Identify critical error cases requiring validation (empty labels,
 *  null callbacks, invalid indices, empty menu activation)
 *  Verify test coverage completeness and edge case handling
 *
 */

#include "../../../third-party/Catch/single_include/catch2/catch.hpp"
#include "../../../source/Interfaces/GUI/Menu.hpp"

using namespace cse498;

TEST_CASE("Menu Constructor", "[Menu]"){
  Menu menu;

  CHECK(!menu.GetSelectedIndex().has_value());
  CHECK(menu.IsEmpty() == true);
  CHECK(menu.GetOptionCount() == 0);
}

TEST_CASE("Add option", "[Menu]"){
  Menu menu;
  bool callback_called = false;

  menu.AddOption("Start Game", [&callback_called]() {
    callback_called = true;
  });

  CHECK(menu.GetSelectedIndex().has_value());
  CHECK(menu.GetSelectedIndex().value() == 0);
  CHECK(menu.GetOptionCount() == 1);
  CHECK(menu.GetOptionLabel(0).value() == "Start Game");
}

TEST_CASE("Add multiple options", "[Menu]") {
  Menu menu;
  menu.AddOption("Option 1", [](){});
  menu.AddOption("Option 2", [](){});
  menu.AddOption("Option 3", [](){});

  CHECK(menu.GetOptionCount() == 3);
  CHECK(menu.GetOptionLabel(0).value() == "Option 1");
  CHECK(menu.GetOptionLabel(1).value() == "Option 2");
  CHECK(menu.GetOptionLabel(2).value() == "Option 3");
}

TEST_CASE("Navigation - select next", "[Menu]") {
  Menu menu;
  menu.AddOption("First", [](){});
  menu.AddOption("Second", [](){});
  menu.AddOption("Third", [](){});

  CHECK(menu.GetSelectedIndex().value() == 0);

  menu.SelectNext();
  CHECK(menu.GetSelectedIndex().value() == 1);

  menu.SelectNext();
  CHECK(menu.GetSelectedIndex().value() == 2);

  // Testing wrap around
  menu.SelectNext();
  CHECK(menu.GetSelectedIndex().value() == 0);
}

TEST_CASE("Navigation - select previous", "[Menu]") {
  Menu menu;
  menu.AddOption("First", [](){});
  menu.AddOption("Second", [](){});
  menu.AddOption("Third", [](){});

  CHECK(menu.GetSelectedIndex().value() == 0);

  // Wrap to last
  menu.SelectPrevious();
  CHECK(menu.GetSelectedIndex().value() == 2);

  menu.SelectPrevious();
  CHECK(menu.GetSelectedIndex().value() == 1);

  menu.SelectPrevious();
  CHECK(menu.GetSelectedIndex().value() == 0);
}

TEST_CASE("Select specific option by index", "[Menu]")
{
  Menu menu;
  menu.AddOption("First", [](){});
  menu.AddOption("Second", [](){});
  menu.AddOption("Third", [](){});

  menu.SelectOption(1);
  CHECK(menu.GetSelectedIndex().value() == 1);

  menu.SelectOption(2);
  CHECK(menu.GetSelectedIndex().value() == 2);

  menu.SelectOption(0);
  CHECK(menu.GetSelectedIndex().value() == 0);
}

TEST_CASE("Navigation - combined selection methods", "[Menu]")
{
    Menu menu;
    menu.AddOption("First", [](){});
    menu.AddOption("Second", [](){});
    menu.AddOption("Third", [](){});
    menu.AddOption("Fourth", [](){});

    // Start at 0
    CHECK(menu.GetSelectedIndex().value() == 0);

    // down with selectNext
    menu.SelectNext();
    CHECK(menu.GetSelectedIndex().value() == 1);

    // index 3
    menu.SelectOption(3);
    CHECK(menu.GetSelectedIndex().value() == 3);

    // move back up
    menu.SelectPrevious();
    CHECK(menu.GetSelectedIndex().value() == 2);

    //back to start with selectOption
    menu.SelectOption(0);
    CHECK(menu.GetSelectedIndex().value() == 0);

    menu.SelectPrevious();
    CHECK(menu.GetSelectedIndex().value() == 3);
}

// callbacks executed correctly when menu options are activated
TEST_CASE("Callback execution", "[Menu]")
{
  Menu menu;
  int counter = 0;

  menu.AddOption("Increment", [&counter]() {
      counter++;
  });

  menu.AddOption("Double", [&counter]() {
      counter *= 2;
  });

  // Test first callback
  menu.SelectOption(0);
  menu.ActivateSelected();
  CHECK(counter == 1);

  // Test second callback
  menu.SelectOption(1);
  menu.ActivateSelected();
  CHECK(counter == 2);
}

TEST_CASE("Remove option", "[Menu]")
{
  Menu menu;
  menu.AddOption("Keep", [](){});
  menu.AddOption("Remove", [](){});
  menu.AddOption("Also Keep", [](){});

  CHECK(menu.GetOptionCount() == 3);

  bool removed = menu.RemoveOption("Remove");
  CHECK(removed == true);
  CHECK(menu.GetOptionCount() == 2);
  CHECK(menu.GetOptionLabel(0).value() == "Keep");
  CHECK(menu.GetOptionLabel(1).value() == "Also Keep");

  // Try to remove non-existent option
  bool not_found = menu.RemoveOption("Doesn't Exist");
  CHECK(not_found == false);
}

TEST_CASE("Remove option updates selection", "[Menu]")
{
  Menu menu;
  menu.AddOption("First", [](){});
  menu.AddOption("Second", [](){});
  menu.AddOption("Third", [](){});

  // Select the last option
  menu.SelectOption(2);
  CHECK(menu.GetSelectedIndex().value() == 2);

  // Remove the last option
  menu.RemoveOption("Third");

  // Selection should now be at index 1 (last valid index)
  CHECK(menu.GetSelectedIndex().value() == 1);
}

TEST_CASE("Handle input - navigation", "[Menu]")
{
  Menu menu;
  menu.AddOption("First", [](){});
  menu.AddOption("Second", [](){});

  // Simulate down arrow (code 2)
  menu.HandleInput(cse498::Menu::InputCode::down);
  CHECK(menu.GetSelectedIndex().value() == 1);

  // Simulate up arrow (code 1)
  menu.HandleInput(cse498::Menu::InputCode::up);
  CHECK(menu.GetSelectedIndex().value() == 0);
}

TEST_CASE("Handle SDL input - navigation", "[Menu]")
{
    Menu menu;

    menu.AddOption("First", [](){});
    menu.AddOption("Second", [](){});

    CHECK(menu.GetSelectedIndex().value() == 0);

    // simulate pressing the down key
    SDL_KeyboardEvent down_event;
    down_event.keysym.sym = SDLK_DOWN;
    menu.HandleSDLInput(down_event);
    CHECK(menu.GetSelectedIndex().value() == 1);

    // simluate pressing the up arrow key
    SDL_KeyboardEvent up_event;
    up_event.keysym.sym = SDLK_UP;
    menu.HandleSDLInput(up_event);
    CHECK(menu.GetSelectedIndex().value() == 0);

    // simulate pressing enter
    bool activated = false;
    menu.Clear();
    menu.AddOption("Press Me", [&activated]() { activated = true; });
    SDL_KeyboardEvent enter_event;
    enter_event.keysym.sym = SDLK_RETURN;
    menu.HandleSDLInput(enter_event);
    CHECK(activated == true);
}

TEST_CASE("Handle input - activation", "[Menu]")
{
  Menu menu;
  bool activated = false;

  menu.AddOption("First", [&activated]() { activated = true; });
  menu.AddOption("Second", [](){});

  // Simulate enter (code 3)
  menu.HandleInput(cse498::Menu::InputCode::enter);
  CHECK(activated == true);
}

TEST_CASE("Clear menu", "[Menu]")
{
    Menu menu;
    menu.AddOption("Test 1", [](){});
    menu.AddOption("Test 2", [](){});

    CHECK(menu.IsEmpty() == false);
    CHECK(menu.GetOptionCount() == 2);

    menu.Clear();

    CHECK(menu.IsEmpty() == true);
    CHECK(menu.GetOptionCount() == 0);
    CHECK(!menu.GetSelectedIndex().has_value());
}

TEST_CASE("Get option label", "[Menu]")
{
    Menu menu;
    menu.AddOption("Start", [](){});
    menu.AddOption("Options", [](){});
    menu.AddOption("Quit", [](){});

    CHECK(menu.GetOptionLabel(0).value() == "Start");
    CHECK(menu.GetOptionLabel(1).value() == "Options");
    CHECK(menu.GetOptionLabel(2).value() == "Quit");
}

TEST_CASE("Empty menu behavior", "[Menu]")
{
    Menu menu;

    // Navigation on empty menu should do nothing (not crash)
    menu.SelectNext();
    menu.SelectPrevious();

    CHECK(!menu.GetSelectedIndex().has_value());
    CHECK(menu.IsEmpty() == true);
}

TEST_CASE("Single option wraparound", "[Menu]")
{
    Menu menu;
    menu.AddOption("Only Option", [](){});

    CHECK(menu.GetSelectedIndex().value() == 0);

    // Next should stay at 0
    menu.SelectNext();
    CHECK(menu.GetSelectedIndex().value() == 0);

    // Previous should stay at 0
    menu.SelectPrevious();
    CHECK(menu.GetSelectedIndex().value() == 0);
}

// Prevent adding menu options with no text
TEST_CASE("Error handling - empty label", "[Menu]")
{
    Menu menu;
    CHECK(menu.AddOption("", [](){}) == false);
}

// Prevent adding menu options that don't do anything
TEST_CASE("Error handling - null callback", "[Menu]")
{
    Menu menu;
    CHECK(menu.AddOption("Test", nullptr) == false);
}

// Changing menu option to index that exists
TEST_CASE("Error handling - invalid index selection", "[Menu]")
{
    Menu menu;
    menu.AddOption("Valid", [](){});
    CHECK(menu.SelectOption(5) == false);
}

// Reads/returns proper label of a menu option
TEST_CASE("Error handling - get invalid label index", "[Menu]")
{
    Menu menu;
    menu.AddOption("Valid", [](){});
    CHECK(menu.GetOptionLabel(10) == std::nullopt);
}

// Preventing activating a callback when nothing to activate (empty menu)
TEST_CASE("Error handling - activate with no selection", "[Menu]")
{
    Menu menu;
    CHECK(menu.ActivateSelected() == false);
}

TEST_CASE("DrawMenu - empty menu does nothing", "[Menu][DrawMenu]")
{
    Menu menu;

    // Should not crash with empty menu
    CHECK_NOTHROW(menu.DrawMenu(nullptr, 0, 0, 400, 300));

    // Menu should still be empty after DrawMenu
    CHECK(menu.IsEmpty() == true);
    CHECK(menu.GetOptionCount() == 0);
}

TEST_CASE("DrawMenu - null renderer handled gracefully", "[Menu][DrawMenu]")
{
    Menu menu;
    menu.AddOption("Test Option", [](){});

    // Should not crash with null renderer
    CHECK_NOTHROW(menu.DrawMenu(nullptr, 0, 0, 400, 300));

    // Menu state should be unchanged
    CHECK(menu.GetOptionCount() == 1);
    CHECK(menu.GetSelectedIndex().has_value());
    CHECK(menu.GetOptionLabel(0).value() == "Test Option");
}

TEST_CASE("DrawMenu - with valid SDL renderer", "[Menu][DrawMenu]")
{
    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Test", 0, 0, 1, 1, SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    Menu menu;
    menu.AddOption("Option 1", [](){});
    menu.AddOption("Option 2", [](){});
    menu.AddOption("Option 3", [](){});

    // Should not crash with valid renderer
    CHECK_NOTHROW(menu.DrawMenu(renderer, 100, 100, 400, 300));

    // Menu state should remain valid
    CHECK(menu.GetOptionCount() == 3);
    CHECK(menu.GetSelectedIndex().value() == 0);

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

TEST_CASE("DrawMenu - after navigation and modifications", "[Menu][DrawMenu]")
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_Window* window = SDL_CreateWindow("Test", 0, 0, 1, 1, SDL_WINDOW_HIDDEN);
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);

    Menu menu;
    menu.AddOption("First", [](){});
    menu.AddOption("Second", [](){});
    menu.AddOption("Third", [](){});

    // DrawMenu initial state
    CHECK_NOTHROW(menu.DrawMenu(renderer, 0, 0, 400, 300));
    CHECK(menu.GetSelectedIndex().value() == 0);

    // Navigate and DrawMenu again
    menu.SelectNext();
    CHECK_NOTHROW(menu.DrawMenu(renderer, 0, 0, 400, 300));
    CHECK(menu.GetSelectedIndex().value() == 1);

    // Remove option and DrawMenu
    menu.RemoveOption("Second");
    CHECK_NOTHROW(menu.DrawMenu(renderer, 0, 0, 400, 300));
    CHECK(menu.GetOptionCount() == 2);

    // Clear and DrawMenu
    menu.Clear();
    CHECK_NOTHROW(menu.DrawMenu(renderer, 0, 0, 400, 300));
    CHECK(menu.IsEmpty() == true);

    // Cleanup
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
