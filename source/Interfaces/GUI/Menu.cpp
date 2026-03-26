/**
 * @file Menu.cpp
 * @author Anagha Jammalamadaka
 *
 *  AI Disclaimer:
 *  - Verify correctness of callback handling and navigation logic
 * - Ensure proper exception handling (std::invalid_argument,
 *   std::out_of_range, std::runtime_error)
 * - Debug compilation issues and build system configuration
 * - Explain C++ concepts (std::function, RAII, memory management)
 */

#include "Menu.hpp"
#include "Text.hpp"
using namespace cse498;

/**
 * Constructor
 * creates empty menu
 */
Menu::Menu() : selected_index(std::nullopt) {}

/**
 * Adds a new menu option with its assoicated action
 * @param label what the user sees (e.g. "start game")
 * @param callback what happens when activates
 */
void Menu::add_option(const std::string &label,
                      std::function<void()> callback) {
  if (label.empty()) {
    throw std::invalid_argument("Menu option label cannot be empty");
  }

  if (!callback) {
    throw std::invalid_argument("Menu option callback cannot be null");
  }

  MenuOption option;
  option.label = label;
  option.callback = callback;

  options.push_back(option);

  // if first option, automatically selected
  if (options.size() == 1) {
    selected_index = 0;
  }
}

/**
 * Removes the menu option with the specified label
 * @param label text to match
 * @return true if found and removed, false otherwise
 */
bool Menu::remove_option(const std::string &label) {
  for (size_t i = 0; i < options.size(); i++) {
    if (options[i].label == label) {
      options.erase(options.begin() + static_cast<int>(i));

      if (options.empty()) {
        selected_index = std::nullopt;
      }

      else if (selected_index.has_value() &&
               *selected_index >= options.size()) { // ADD *
        selected_index = options.size() - 1;
      }

      return true;
    }
  }
  return false;
}

/**
 * @return returns total number of options currently in the menu
 */
size_t Menu::get_option_count() const { return options.size(); }

/**
 * @return returns index of the currently selected option or -1 if no option is
 * selected
 */
std::optional<size_t> Menu::get_selected_index() const {
  return selected_index;
}

/**
 * Moves the selection dwon to the next option, wrapping around to first option
 * if current at last
 */
void Menu::select_next() {
  if (options.empty())
    return;

  if (!selected_index.has_value()) {
    selected_index = 0;
  } else {
    selected_index = (*selected_index + 1) % options.size();
  }
}

/**
 * Moves selection up to prev. option, wrapping around to the last option if
 * currently at the first
 */
void Menu::select_previous() {
  if (options.empty())
    return;

  if (!selected_index.has_value()) {
    selected_index = options.size() - 1;
  }

  else if (*selected_index == 0) { // ADD *
    selected_index = options.size() - 1;
  }

  else {
    selected_index = *selected_index - 1; // ADD *
  }
}

/**
 * Sets selections to the option at the specified index
 * @param index 0-based position in the menu
 */
void Menu::select_option(size_t index) {
  if (index >= options.size()) {
    throw std::out_of_range("Menu option index out of range");
  }

  selected_index = static_cast<int>(index);
}

/**
 * Executes callback function of the currently selected option
 */
void Menu::activate_selected() {
  if (selected_index < 0 ||
      selected_index >= static_cast<int>(options.size())) {
    throw std::runtime_error("No option selected");
  }

  options[*selected_index].callback();
}

/**
 * @param index Position of the chosen option
 * @return label of the option at specific index
 */
std::string Menu::get_option_label(size_t index) const {
  if (index >= options.size()) {
    throw std::out_of_range("Menu option index out of range");
  }

  return options[index].label;
}

/**
 * @return true if menu has no options, false otherwise
 */
bool Menu::is_empty() const noexcept { return options.empty(); }

/**
 * Removes all options from the menu and resets the selection to -1 ( when
 * nothing is selected)
 */
void Menu::clear() {
  options.clear();
  selected_index = std::nullopt;
}

/**
 * Handles user inputted keyboard option
 * @param input_code values for keyboard functions
 */
void Menu::handle_input(InputCode input_code) {
  switch (input_code) {

  case InputCode::up:
    select_previous();
    break;

  case InputCode::down:
    select_next();
    break;

  case InputCode::enter:
    if (!is_empty()) {
      activate_selected();
    }

    break;
  }
}

void Menu::draw(SDL_Renderer *renderer, int x, int y,
                [[maybe_unused]] int width, int height) {
  // Return silently if menu is empty - nothing to draw
  if (is_empty()) {
    return;
  }

  // Return silently if no renderer provided
  if (!renderer) {
    return;
  }

  // Create Text object for rendering
  Text menuText(renderer);
  menuText.SetSize(18); // Font size for menu options

  // Calculate spacing between options
  int option_count = static_cast<int>(get_option_count());
  int spacing = (option_count > 1) ? height / option_count : height;

  // Draw each menu option
  int current_y = y;

  for (size_t i = 0; i < get_option_count(); i++) {
    // Set content
    menuText.SetContent(get_option_label(i));

    // Check if this option is selected
    bool is_selected = false;
    if (selected_index.has_value() && *selected_index == i) {
      is_selected = true;
    }

    // Set color based on selection
    if (is_selected) {
      // Yellow for selected option
      menuText.SetColor(255, 255, 0, 255);
      menuText.SetBold(true);
    } else {
      // White for unselected options
      menuText.SetColor(255, 255, 255, 255);
      menuText.SetBold(false);
    }

    // Draw the option
    menuText.Draw(x, current_y);

    // Move to next line
    current_y += spacing;
  }
}