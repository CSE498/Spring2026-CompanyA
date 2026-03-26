/**
 * @file Menu.hpp
 * @author Anagha Jammalamadaka
 */

#ifndef MENU_HPP
#define MENU_HPP

#include <functional>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include "Text.hpp"

namespace cse498 {

class Menu {
private:
  struct MenuOption {
    std::string label;
    std::function<void()> callback;
  };

  std::vector<MenuOption> options;
  std::optional<size_t> selected_index; // holds -1 if nothing is chosen

public:
  Menu();

  /**
   * add new option to menu
   * label The display text for option
   * callback Function to call when option is chosen
   */
  void add_option(const std::string &label, std::function<void()> callback);

  bool remove_option(const std::string &label);

  // get the number of options in th menu
  // return # of options
  size_t get_option_count() const;

  // get index of selected option
  // returns index of selected option, or -1 if nothing is chosen
  std::optional<size_t> get_selected_index() const;

  // move selection to the next option
  void select_next();

  // move selection to the previous option
  void select_previous();

  // selects option by it's index
  // index: index to select
  void select_option(size_t index);

  // activates selected options, triggers its callback
  void activate_selected();

  // gets the label of an option by index
  // index The option's index
  // returns the label's string
  // std::out_of_range if index is invalid
  std::string get_option_label(size_t index) const;

  // check if menu has options
  // true if menu empty
  bool is_empty() const noexcept;

  // clear all options from the menu
  void clear();

  enum class InputCode { up = 1, down = 2, enter = 3 };
  // handles keyboard/mouse selection
  // input_code: int representing user input (1 = up, 2 = down, 3 = enter)
  void handle_input(InputCode input_code);

  // draw function for menu button
  void draw(SDL_Renderer *renderer, int x, int y, int width, int height);

  ~Menu() = default;
};

} // namespace cse498

#endif // MENU_HPP
