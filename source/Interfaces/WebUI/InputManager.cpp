#ifdef __EMSCRIPTEN__

#include "./InputManager.hpp"
#include "./interface/WebInterface.hpp"

#include <cctype>
#include <string>

using namespace cse498;
using std::string;

/// @brief Convert a string to lower case
/// @param key the string to convert
/// @return the original string in all lower case
std::string ToLower(const std::string& key) {
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

InputManager::~InputManager() {
    emscripten_html5_remove_event_listener(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                           this,
                                           EMSCRIPTEN_EVENT_KEYDOWN,
                                           reinterpret_cast<void*>(InputManager::OnKeyDown));
    emscripten_html5_remove_event_listener(EMSCRIPTEN_EVENT_TARGET_WINDOW,
                                           this,
                                           EMSCRIPTEN_EVENT_KEYUP,
                                           reinterpret_cast<void*>(InputManager::OnKeyUp));
}

EM_BOOL InputManager::OnKeyDown(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* inputManagerPointer) {
    if (eventType != EMSCRIPTEN_EVENT_KEYDOWN || inputManagerPointer == nullptr || keyEvent->repeat)
        return EM_FALSE;

    auto manager = static_cast<InputManager*>(inputManagerPointer);

    auto key = std::string(keyEvent->key);

    std::string lowerKey = ToLower(key);

    // Handle numeric keys for hotbar selection - immediate response
    if (lowerKey.length() == 1 && std::isdigit(lowerKey[0])) {
        int digit = lowerKey[0] - '0';
        size_t slotIndex = (digit == 0) ? 9 : (digit - 1); // 1-9 maps to 0-8, 0 maps to 9
        manager->mInterface.SetPlayerHotbarIndex(slotIndex);
        return EM_FALSE;
    }

    // Handle inventory menu toggle
    if (lowerKey == "i") {
        manager->mInterface.OpenInventory();
        return EM_FALSE;
    }

    // Handle pause button
    if (lowerKey == "escape") {
        manager->mInterface.HandlePause();
        return EM_TRUE; // consume the event to prevent default browser behavior (e.g., exiting fullscreen or opening
                        // browser menu)
    }

    // Add keys for movement and interaction to the front of the pressed keys deque
    // so that it is checked first in GetAction and allows for responsive input
    // even when multiple keys are pressed
    manager->AddKeyPressed(lowerKey);

    return EM_FALSE; // Allow default browser behavior for other keys for accessibility (e.g., tab navigation, browser
                     // shortcuts)
}

EM_BOOL InputManager::OnKeyUp(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* inputManagerPointer) {
    if (eventType != EMSCRIPTEN_EVENT_KEYUP || inputManagerPointer == nullptr)
        return EM_FALSE;

    auto manager = static_cast<InputManager*>(inputManagerPointer);

    auto key = std::string(keyEvent->key);

    std::string lowerKey = ToLower(key);

    // ignore pause/inventory/number button key up
    if (lowerKey == "escape" || lowerKey == "i" || (lowerKey.length() == 1 && std::isdigit(lowerKey[0]))) {
        return EM_TRUE;
    }

    manager->RemoveKeyPressed(lowerKey);

    return EM_TRUE;
}

InputManager::ActiveAction InputManager::GetAction() {
    if (mInterface.IsPaused() || mKeysPressed.empty()) {
        return ActiveAction::None;
    }

    const std::string& key = mKeysPressed.front();

    // clang-format off
    if (key == "w") { return ActiveAction::Up; }
    if (key == "a") { return ActiveAction::Left; }
    if (key == "s") { return ActiveAction::Down; }
    if (key == "d") { return ActiveAction::Right; }
    if (key == "e") { return ActiveAction::Interact; }
    if (key == "q") { return ActiveAction::Quit; }
    return ActiveAction::None;
    // clang-format on
}

#endif
