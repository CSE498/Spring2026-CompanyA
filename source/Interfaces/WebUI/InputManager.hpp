#pragma once

#include <emscripten/html5.h>
#include <string>
#include <ranges>
#include <deque>

namespace cse498 {

class WebInterface;
  
class InputManager {
public:

  InputManager(WebInterface & interface);
  ~InputManager();
    
  enum class ActiveAction {Up, Left, Down, Right, Interact, Pause, Quit, None};

  static EM_BOOL OnKeyDown(int eventType, const EmscriptenKeyboardEvent * keyEvent, void * userData);
  static EM_BOOL OnKeyUp(int eventType, const EmscriptenKeyboardEvent * keyEvent, void * userData);

  ActiveAction GetAction();

  void AddKeyPressed(const std::string & key) { mKeysPressed.push_front(key); }
  void RemoveKeyPressed(const std::string & key) { std::erase(mKeysPressed, key); }
  int IsKeyPressed() const { return !mKeysPressed.empty(); }

  void SetTapAction(std::string & key) { mTapAction = key; }

private:
  WebInterface & mInterface;
  std::deque<std::string> mKeysPressed{};
  std::string mTapAction{};
};

}