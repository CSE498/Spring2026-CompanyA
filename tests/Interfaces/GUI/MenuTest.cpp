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

  CHECK(menu.is_empty() == true);
  CHECK(menu.get_option_count() == 0);
  CHECK(menu.get_selected_index() == -1);
}

TEST_CASE("Add option", "[Menu]"){
  Menu menu;
  bool callback_called = false;

  menu.add_option("Start Game", [&callback_called]() {
    callback_called = true;
  });

  CHECK(menu.get_option_count() == 1);
  CHECK(menu.get_selected_index() == 0);
  CHECK(menu.get_option_label(0) == "Start Game");
}

TEST_CASE("Add multiple options", "[Menu]") {
  Menu menu;
  menu.add_option("Option 1", [](){});
  menu.add_option("Option 2", [](){});
  menu.add_option("Option 3", [](){});

  CHECK(menu.get_option_count() == 3);
  CHECK(menu.get_option_label(0) == "Option 1");
  CHECK(menu.get_option_label(1) == "Option 2");
  CHECK(menu.get_option_label(2) == "Option 3");
}

TEST_CASE("Navigation - select next", "[Menu]") {
  Menu menu;
  menu.add_option("First", [](){});
  menu.add_option("Second", [](){});
  menu.add_option("Third", [](){});

  CHECK(menu.get_selected_index() == 0);

  menu.select_next();
  CHECK(menu.get_selected_index() == 1);

  menu.select_next();
  CHECK(menu.get_selected_index() == 2);

  // Testing wrap around

  menu.select_next();
  CHECK(menu.get_selected_index() == 0);
}

TEST_CASE("Select specific option by index", "[Menu]")
{
  Menu menu;
  menu.add_option("First", [](){});
  menu.add_option("Second", [](){});
  menu.add_option("Third", [](){});

  menu.select_option(1);
  CHECK(menu.get_selected_index() == 1);

  menu.select_option(2);
  CHECK(menu.get_selected_index() == 2);

  menu.select_option(0);
  CHECK(menu.get_selected_index() == 0);
}

// callbacks executed correctly when menu options are activated
TEST_CASE("Callback execution", "[Menu]")
{
  Menu menu;
  int counter = 0;

  menu.add_option("Increment", [&counter]() {
      counter++;
  });

  menu.add_option("Double", [&counter]() {
      counter *= 2;
  });

  // Test first callback
  menu.select_option(0);
  menu.activate_selected();
  CHECK(counter == 1);

  // Test second callback
  menu.select_option(1);
  menu.activate_selected();
  CHECK(counter == 2);
}

TEST_CASE("Remove option", "[Menu]")
{
  Menu menu;
  menu.add_option("Keep", [](){});
  menu.add_option("Remove", [](){});
  menu.add_option("Also Keep", [](){});

  CHECK(menu.get_option_count() == 3);

  bool removed = menu.remove_option("Remove");
  CHECK(removed == true);
  CHECK(menu.get_option_count() == 2);
  CHECK(menu.get_option_label(0) == "Keep");
  CHECK(menu.get_option_label(1) == "Also Keep");

  // Try to remove non-existent option
  bool not_found = menu.remove_option("Doesn't Exist");
  CHECK(not_found == false);
}

TEST_CASE("Remove option updates selection", "[Menu]")
{
  Menu menu;
  menu.add_option("First", [](){});
  menu.add_option("Second", [](){});
  menu.add_option("Third", [](){});

  // Select the last option
  menu.select_option(2);
  CHECK(menu.get_selected_index() == 2);

  // Remove the last option
  menu.remove_option("Third");

  // Selection should now be at index 1 (last valid index)
  CHECK(menu.get_selected_index() == 1);
}

TEST_CASE("Handle input - navigation", "[Menu]")
{
  Menu menu;
  menu.add_option("First", [](){});
  menu.add_option("Second", [](){});

  // Simulate down arrow (code 2)
  menu.handle_input(2);
  CHECK(menu.get_selected_index() == 1);

  // Simulate up arrow (code 1)
  menu.handle_input(1);
  CHECK(menu.get_selected_index() == 0);
}

TEST_CASE("Handle input - activation", "[Menu]")
{
  Menu menu;
  bool activated = false;

  menu.add_option("First", [&activated]() { activated = true; });
  menu.add_option("Second", [](){});

  // Simulate enter (code 3)
  menu.handle_input(3);
  CHECK(activated == true);
}

TEST_CASE("Handle input - unknown code", "[Menu]")
{
  Menu menu;
  menu.add_option("First", [](){});
  menu.add_option("Second", [](){});

  int initial_index = menu.get_selected_index();

  // Simulate unknown input code
  menu.handle_input(999);

  // Selection should not change
  CHECK(menu.get_selected_index() == initial_index);
}

TEST_CASE("Clear menu", "[Menu]")
{
    Menu menu;
    menu.add_option("Test 1", [](){});
    menu.add_option("Test 2", [](){});

    CHECK(menu.is_empty() == false);
    CHECK(menu.get_option_count() == 2);

    menu.clear();

    CHECK(menu.is_empty() == true);
    CHECK(menu.get_option_count() == 0);
    CHECK(menu.get_selected_index() == -1);
}

TEST_CASE("Get option label", "[Menu]")
{
    Menu menu;
    menu.add_option("Start", [](){});
    menu.add_option("Options", [](){});
    menu.add_option("Quit", [](){});

    CHECK(menu.get_option_label(0) == "Start");
    CHECK(menu.get_option_label(1) == "Options");
    CHECK(menu.get_option_label(2) == "Quit");
}

TEST_CASE("Empty menu behavior", "[Menu]")
{
    Menu menu;

    // Navigation on empty menu should do nothing (not crash)
    menu.select_next();
    menu.select_previous();

    CHECK(menu.get_selected_index() == -1);
    CHECK(menu.is_empty() == true);
}

TEST_CASE("Single option wraparound", "[Menu]")
{
    Menu menu;
    menu.add_option("Only Option", [](){});

    CHECK(menu.get_selected_index() == 0);

    // Next should stay at 0
    menu.select_next();
    CHECK(menu.get_selected_index() == 0);

    // Previous should stay at 0
    menu.select_previous();
    CHECK(menu.get_selected_index() == 0);
}

// Prevent adding menu options with no text
TEST_CASE("Error handling - empty label", "[Menu]")
{
    Menu menu;

    REQUIRE_THROWS_AS(
        menu.add_option("", [](){}),
        std::invalid_argument
    );
}

// Prevent adding menu options that don't do anything
TEST_CASE("Error handling - null callback", "[Menu]")
{
    Menu menu;

    REQUIRE_THROWS_AS(
        menu.add_option("Test", nullptr),
        std::invalid_argument
    );
}

// Changing menu option to index that exists
TEST_CASE("Error handling - invalid index selection", "[Menu]")
{
    Menu menu;
    menu.add_option("Valid", [](){});

    REQUIRE_THROWS_AS(
        menu.select_option(5),
        std::out_of_range
    );
}

// Reads/returns proper label of a menu option
TEST_CASE("Error handling - get invalid label index", "[Menu]")
{
    Menu menu;
    menu.add_option("Valid", [](){});

    REQUIRE_THROWS_AS(
        menu.get_option_label(10),
        std::out_of_range
    );
}

// Preventing activating a callback when nothing to activate (empty menu)
TEST_CASE("Error handling - activate with no selection", "[Menu]")
{
    Menu menu;

    REQUIRE_THROWS_AS(
        menu.activate_selected(),
        std::runtime_error
    );
}
