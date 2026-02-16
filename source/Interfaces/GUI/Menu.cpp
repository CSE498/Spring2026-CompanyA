/**
 * @file Menu.cpp
 * @author Anagha Jammalamadaka
 */
 
#include "Menu.hpp"
using namespace cse498;

 // no selection
Menu::Menu() : selected_index(-1) {}

void Menu::add_option(const std::string& label, std::function<void()> callback) {
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

bool Menu::remove_option(const std::string& label) {
  for(size_t i = 0; i < options.size(); i++){
    if (options[i].label == label) {
      options.erase(options.begin() + i);

      // Update selection index
      if (options.empty()){
        selected_index = -1;
      }

      else if (selected_index >= static_cast<int>(options.size())){
        selected_index = options.size() -1;
      }

      return true;
    }

  }
  return false;

}

size_t Menu::get_option_count() const {
  return options.size();
}

int Menu::get_selected_index() const {
  return selected_index;
}

void Menu::select_next() {
  if(options.empty())
      return;

  selected_index = (selected_index + 1) % options.size();
}

void Menu::select_previous() {
  if (options.empty())
    return;

  selected_index--;
  if (selected_index < 0){
    selected_index = options.size() - 1;
  }
}

void Menu::select_option(size_t index) {
  if (index >= options.size()) {
    throw std::out_of_range("Menu option index out of range");
  }

  selected_index = static_cast<int>(index);
}

void Menu::activate_selected() {
  if (selected_index < 0 || selected_index >= static_cast<int>(options.size())) {
    throw std::runtime_error("No option selected");
  }

  options[selected_index].callback();
}


std::string Menu::get_option_label(size_t index) const {
  if (index >= options.size()) {
    throw std::out_of_range("Menu option index out of range");
  }

  return options[index].label;
}

bool Menu::is_empty() const {
  return options.empty();
}

void Menu::clear() {
  options.clear();
  selected_index = -1;
}

void Menu::handle_input(int input_code) {
  const int up_arrow = 1;
  const int down_arrow = 2;
  const int enter = 3;

  switch(input_code) {
    case up_arrow:
      select_previous();
      break;
    case down_arrow:
        select_next();
        break;
    case enter:
      if (!is_empty()) {
        activate_selected();
      }
      break;
    default:
        // if input is unknown
        break;
  }
}

// void Menu::draw(int x, int y, int width, int height) {
//   // TO DO
// }