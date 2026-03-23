#ifdef __EMSCRIPTEN__

#include "./InputManager.hpp"
#include "./interface/WebInterface.hpp"

#include <string>

using namespace cse498;
using std::string;

InputManager::InputManager(WebInterface & interface) : mInterface(interface) {
  emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, InputManager::OnKeyDown);
  emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, InputManager::OnKeyUp);
}

InputManager::~InputManager() {
  emscripten_html5_remove_all_event_listeners();
}

EM_BOOL InputManager::OnKeyDown(int eventType, const EmscriptenKeyboardEvent * keyEvent, void * userData) {
  if (eventType != EMSCRIPTEN_EVENT_KEYDOWN || userData == nullptr || keyEvent->repeat) return EM_FALSE;

  auto manager = static_cast<InputManager *>(userData);

  auto key = std::string(keyEvent->key);

  // special case pause button
  if (key == "Escape") {
    manager->mInterface.HandlePause();
    return EM_TRUE;
  }

  if (!manager->IsKeyPressed()) manager->SetTapAction(key);
  manager->AddKeyPressed(key);

  return EM_TRUE;
}

EM_BOOL InputManager::OnKeyUp(int eventType, const EmscriptenKeyboardEvent * keyEvent, void * userData) {
  if (eventType != EMSCRIPTEN_EVENT_KEYUP || userData == nullptr) return EM_FALSE;

  auto manager = static_cast<InputManager *>(userData);

  auto key = std::string(keyEvent->key);

  // ignore pause button key up
  if (key == "Escape") {
    return EM_TRUE;
  }

  manager->RemoveKeyPressed(key);

  return EM_TRUE;
}

InputManager::ActiveAction InputManager::GetAction() {
  if (mInterface.IsPaused()) return ActiveAction::None;

  // if no keys currently pressed, check the tap action
  //  to see if they just tapped a key while input was blocked 
  // then set to empty string so it is only done once
  std::string key;
  if (mKeysPressed.empty()) {
    key = mTapAction;
    mTapAction = "";
  }
  else {
    key = mKeysPressed.front(); 
  }

  std::string lowerKey = key
      | std::ranges::views::transform([](unsigned char c) { 
          return std::tolower(c);
        }) 
      | std::ranges::to<std::string>();

  if (lowerKey == "w") return ActiveAction::Up;
  if (lowerKey == "a") return ActiveAction::Left;
  if (lowerKey == "s") return ActiveAction::Down;
  if (lowerKey == "d") return ActiveAction::Right;
  if (lowerKey == "e") return ActiveAction::Interact;
  if (lowerKey == "q") return ActiveAction::Quit;
  if (lowerKey == "escape") return ActiveAction::Pause;
  return ActiveAction::None;
}

#endif