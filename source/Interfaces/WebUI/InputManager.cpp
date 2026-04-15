#ifdef __EMSCRIPTEN__

#include "./InputManager.hpp"
#include "./interface/WebInterface.hpp"

#include <string>

using namespace cse498;
using std::string;

/// @brief Convert a string to lower case
/// @param key the string to convert
/// @return the original string in all lower case
std::string ToLower(std::string& key) {
    // clang-format off
    std::string lowerKey =
        key
        | std::ranges::views::transform([](unsigned char c) { return std::tolower(c); })
        | std::ranges::to<std::string>()
    ;
    return lowerKey;
    // clang-format on
}

InputManager::InputManager(WebInterface& interface) : mInterface(interface) {
    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, InputManager::OnKeyDown);
    emscripten_set_keyup_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, this, true, InputManager::OnKeyUp);
}

InputManager::~InputManager() { emscripten_html5_remove_all_event_listeners(); }

EM_BOOL InputManager::OnKeyDown(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData) {
    if (eventType != EMSCRIPTEN_EVENT_KEYDOWN || userData == nullptr || keyEvent->repeat)
        return EM_FALSE;

    auto manager = static_cast<InputManager*>(userData);

    auto key = std::string(keyEvent->key);

    std::string lowerKey = ToLower(key);

    // special case pause button
    if (lowerKey == "escape") {
        manager->mInterface.HandlePause();
        return EM_TRUE;
    }

    if (!manager->IsKeyPressed())
        manager->SetTapAction(lowerKey);
    manager->AddKeyPressed(lowerKey);

    return EM_TRUE;
}

EM_BOOL InputManager::OnKeyUp(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* userData) {
    if (eventType != EMSCRIPTEN_EVENT_KEYUP || userData == nullptr)
        return EM_FALSE;

    auto manager = static_cast<InputManager*>(userData);

    auto key = std::string(keyEvent->key);

    std::string lowerKey = ToLower(key);

    // ignore pause button key up
    if (lowerKey == "escape") {
        return EM_TRUE;
    }

    manager->RemoveKeyPressed(lowerKey);

    return EM_TRUE;
}

InputManager::ActiveAction InputManager::GetAction() {
    if (mInterface.IsPaused()) {
        mTapAction = "";
        return ActiveAction::None;
    }

    // if no keys currently pressed, check the tap action
    // to see if they just tapped a key while input was blocked
    // then set to empty string so it is only done once
    std::string key;
    mKeysPressed.empty() ? key = mTapAction : key = mKeysPressed.front();
    mTapAction = "";

    // clang-format off
    if (key == "w") { return ActiveAction::Up; }
    if (key == "a") { return ActiveAction::Left; }
    if (key == "s") { return ActiveAction::Down; }
    if (key == "d") { return ActiveAction::Right; }
    if (key == "e") { return ActiveAction::Interact; }
    if (key == "q") { return ActiveAction::Quit; }
    if (key == "escape") { return ActiveAction::Pause; }
    return ActiveAction::None;
    // clang-format on
}

#endif
