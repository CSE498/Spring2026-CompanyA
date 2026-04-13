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

#include <algorithm>
using namespace cse498;

/**
 * Constructor
 * creates empty menu
 */
Menu::Menu() : mSelectedIndex(std::nullopt) {}

/**
 *  Adds a new menu option with its associated action
 * @param label what the user sees (e.g. "start game")
 * @param callback what happens when activates
 * @return
 */

bool Menu::AddOption(const std::string &label, std::function<void()> callback)
{
    if (label.empty() || !callback)
    {
        return false;
    }

    // prevents duplicate labels
    const bool duplicate = std::any_of(mOptions.begin(), mOptions.end(),
                                       [&label](const MenuOption &option) { return option.label == label; });

    if (duplicate)
    {
        return false;
    }

    mOptions.push_back({label, std::move(callback)}); // move instead of copy

    if (mOptions.size() == 1)
    {
        mSelectedIndex = 0;
    }

    return true;
}

/**
 * Removes the menu option with the specified label
 * @param label text to match
 * @return true if found and removed, false otherwise
 */

bool Menu::RemoveOption(const std::string &label)
{
    // when you build, code runs fine. If you see underlined red, just IDE issue
    auto it =
        std::find_if(mOptions.begin(), mOptions.end(), [&label](const MenuOption &opt) { return opt.label == label; });

    if (it == mOptions.end())
        return false;

    size_t i = static_cast<size_t>(std::distance(mOptions.begin(), it));
    mOptions.erase(it);

    if (mOptions.empty())
    {
        mSelectedIndex = std::nullopt;
    }

    else if (mSelectedIndex.has_value())
    {

        if (*mSelectedIndex >= mOptions.size())
        {
            mSelectedIndex = mOptions.size() - 1;
        }

        else if (i < *mSelectedIndex)
        {
            mSelectedIndex = *mSelectedIndex - 1;
        }
    }

    return true;
}

/**
 * @return returns total number of options currently in the menu
 */
size_t Menu::GetOptionCount() const { return mOptions.size(); }

/**
 * @return returns index of the currently selected option or -1 if no option is selected
 */
std::optional<size_t> Menu::GetSelectedIndex() const { return mSelectedIndex; }

/**
 * Moves the selection down to the next option, wrapping around to first option if current at last
 */
void Menu::SelectNext()
{
    if (mOptions.empty())
        return;

    if (!mSelectedIndex.has_value())
    {
        mSelectedIndex = 0;
    }

    else
    {
        mSelectedIndex = (*mSelectedIndex + 1) % mOptions.size();
    }
}

/**
 * Moves selection up to prev. option, wrapping around to the last option if
 * currently at the first
 */
void Menu::SelectPrevious()
{
    if (mOptions.empty())
        return;

    if (!mSelectedIndex.has_value())
    {
        mSelectedIndex = mOptions.size() - 1;
    }

    else if (*mSelectedIndex == 0)
    { // ADD *
        mSelectedIndex = mOptions.size() - 1;
    }

    else
    {
        mSelectedIndex = *mSelectedIndex - 1; // ADD *
    }
}

/**
 * Sets selections to the option at the specified index
 * @param index 0-based position in the menu
 * @return false if the selected index is larger than size
 */
bool Menu::SelectOption(size_t index)
{
    if (index >= mOptions.size())
    {
        return false;
    }

    mSelectedIndex = index;
    return true;
}

/**
 * Executes callback function of the currently selected option
 */
bool Menu::ActivateSelected() const
{
    if (!mSelectedIndex.has_value() || *mSelectedIndex >= mOptions.size())
    {
        return false;
    }

    mOptions.at(*mSelectedIndex).callback();
    return true;
}

/**
 * @param index Position of the chosen option
 * @return label of the option at specific index
 */
std::optional<std::string> Menu::GetOptionLabel(size_t index) const
{
    if (index >= mOptions.size())
    {
        return std::nullopt;
    }

    return mOptions.at(index).label;
}

/**
 * @return true if menu has no options, false otherwise
 */
bool Menu::IsEmpty() const noexcept { return mOptions.empty(); }

/**
 * Removes all options from the menu and resets the selection
 */
void Menu::Clear()
{
    mOptions.clear();
    mSelectedIndex = std::nullopt;
}

/**
 * Handles menu responses to input codes
 * @param input_code input code values
 */
void Menu::HandleInput(InputCode input_code)
{
    switch (input_code)
    {

    case InputCode::up:
        SelectPrevious();
        break;

    case InputCode::down:
        SelectNext();
        break;


    case InputCode::enter:
        if (!IsEmpty())
        {
            ActivateSelected();
        }

        break;
    }
}

/**
 * Function that takes an actual SDL keyboard event and translates it into InputCode
 * @param key_event SDL releated up/down/enter functionality
 */
void Menu::HandleSDLInput(const SDL_KeyboardEvent &key_event)
{
    switch (key_event.keysym.sym)
    {

    case SDLK_UP:
        HandleInput(InputCode::up);
        break;

    case SDLK_DOWN:
        HandleInput(InputCode::down);
        break;

    case SDLK_RETURN:
        HandleInput(InputCode::enter);
        break;

    default:
        break;
    }
}


/**
 * Renders to menu options to the screen at the specified postion
 * @param renderer SDL_Renderer used to draw the menu
 * @param x The x coordinate of the top left corner of the menu
 * @param y The y coordinate of the top left corner of the menu
 * @param width The width of the menu area (unused for now)
 * @param height The height of the menu area, calculates spacing between options
 */
void Menu::DrawMenu(SDL_Renderer *renderer, int x, int y, [[maybe_unused]] int width, int height,
                    const MenuStyle &style)
{
    if (IsEmpty() || !renderer)
        return;

    Text menuText(renderer);
    menuText.SetSize(style.font_size);

    int option_count = static_cast<int>(GetOptionCount());
    int spacing = (option_count > 1) ? height / option_count : height;

    // Lambda captures selection state — avoids repeating the check each iteration
    auto isSelected = [&](size_t i) { return mSelectedIndex.has_value() && *mSelectedIndex == i; };

    int current_y = y;
    for (size_t i = 0; i < GetOptionCount(); i++)
    {
        menuText.SetContent(GetOptionLabel(i).value());

        if (isSelected(i))
        {
            menuText.SetColor(style.selected_color);
            menuText.SetBold(style.bold_selected);
        }

        else
        {
            menuText.SetColor(style.unselected_color);
            menuText.SetBold(false);
        }

        menuText.Draw(x, current_y);
        current_y += spacing;
    }
}