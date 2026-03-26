#pragma once

#include <emscripten/html5.h>
#include <string>
#include <ranges>
#include <deque>

namespace cse498 {

class WebInterface;
  
/// @class InputManager
/// @brief Manages user input for the web interface.
/// @details Handles keyboard events and translates them into game actions.
class InputManager {
public:

  /// @brief Constructs an InputManager.
  /// @param interface Reference to the WebInterface.
  InputManager(WebInterface & interface);
  ~InputManager();
    
  /// @enum ActiveAction
  /// @brief Enumeration of possible user actions.
  enum class ActiveAction {Up, Left, Down, Right, Interact, Pause, Quit, None};

  /// @brief Callback for key down events.
  /// @param eventType The event type.
  /// @param keyEvent The keyboard event.
  /// @param userData User data pointer.
  /// @return EM_BOOL indicating if the event was handled.
  static EM_BOOL OnKeyDown(int eventType, const EmscriptenKeyboardEvent * keyEvent, void * userData);

  /// @brief Callback for key up events.
  /// @param eventType The event type.
  /// @param keyEvent The keyboard event.
  /// @param userData User data pointer.
  /// @return EM_BOOL indicating if the event was handled.
  static EM_BOOL OnKeyUp(int eventType, const EmscriptenKeyboardEvent * keyEvent, void * userData);

  /// @brief Gets the current active action.
  /// @return The active action.
  ActiveAction GetAction();

  /// @brief Adds a pressed key to the deque.
  /// @param key The key string.
  void AddKeyPressed(const std::string & key) { mKeysPressed.push_front(key); }

  /// @brief Removes a released key from the deque.
  void RemoveKeyPressed(const std::string & key) { std::erase(mKeysPressed, key); }

  /// @brief Checks if any key is pressed.
  /// @return True if keys are pressed.
  int IsKeyPressed() const { return !mKeysPressed.empty(); }

  /// @brief Sets the tap action.
  /// @param key The key for tap action.
  void SetTapAction(std::string & key) { mTapAction = key; }

private:
  /// @brief Reference to the WebInterface.
  WebInterface & mInterface;

  /// @brief Deque of currently pressed keys.
  std::deque<std::string> mKeysPressed{};

  /// @brief Current tap action key.
  std::string mTapAction{};
};

}