/**
 * @file Menu.hpp
 * @author Anagha Jammalamadaka
 */
 
#ifndef MENU_HPP
#define MENU_HPP

#include <string>
#include <vector>
#include <functional>
#include <stdexcept>
#include <optional>

#include "Text.hpp"

namespace cse498 {

struct MenuStyle
{

  static constexpr int kDefaulFontSize = 24;
  static constexpr bool kDefautlBoldSelected = true;

  int font_size = kDefaulFontSize;
  bool bold_selected = kDefautlBoldSelected;


  Color selected_color = {255, 255, 0, 255}; // yellow
  Color unselected_color = {255, 255, 255, 255 }; // white

};

class Menu {
private:

  struct MenuOption {
    std::string label;
    std::function<void()> callback;
   };

   std::vector<MenuOption> mOptions;
   std::optional<size_t> mSelectedIndex; // holds -1 if nothing is chosen

public:

  Menu();

  enum class InputCode
  {
    up = 1,
    down = 2,
    enter = 3
};

  bool AddOption(const std::string& label, std::function<void()> callback);

  bool RemoveOption(const std::string& label);

  size_t GetOptionCount() const;

  std::optional<size_t> GetSelectedIndex() const;

  void SelectNext();

  void SelectPrevious();

  bool SelectOption(size_t index);

  bool ActivateSelected() const;

  std::optional <std::string> GetOptionLabel(size_t index) const;

  bool IsEmpty() const noexcept;

  void Clear();

  void HandleInput(InputCode input_code);

  void DrawMenu(SDL_Renderer* renderer, int x, int y, int width, int height, const MenuStyle& style = MenuStyle{});

  void HandleSDLInput(const SDL_KeyboardEvent& key_event);


  ~Menu() = default;
};

}

#endif //MENU_HPP
