#pragma once

#include <deque>
#include <emscripten/html5.h>
#include <ranges>
#include <string>

namespace cse498 {

class WebInterface;

/// @class InputManager
/// @brief Manages user input for the web interface.
/// @details Handles keyboard events and translates them into game actions.
class InputManager {
public:
    /// @brief Constructs an InputManager.
    /// @param interface Reference to the WebInterface.
    InputManager(WebInterface& interface);
    ~InputManager();

    /// @enum ActiveAction
    /// @brief Enumeration of possible user actions.
    enum class ActiveAction { Up, Left, Down, Right, Interact, Quit, None };

    /// @brief Callback for key down events.
    /// @param eventType The event type (should be EMSCRIPTEN_EVENT_KEYDOWN).
    /// @param keyEvent The keyboard event.
    /// @param inputManagerPointer Pointer to this InputManager instance. This is needed
    ///                            to access the instance from the static callback function.
    ///                            It must be void * to match the expected signature of the Emscripten callback.
    /// @return EM_BOOL indicating if the event was handled.
    static EM_BOOL OnKeyDown(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* inputManagerPointer);

    /// @brief Callback for key up events.
    /// @param eventType The event type (should be EMSCRIPTEN_EVENT_KEYUP).
    /// @param keyEvent The keyboard event.
    /// @param inputManagerPointer Pointer to this InputManager instance. This is needed
    ///                            to access the instance from the static callback function.
    ///                            It must be void * to match the expected signature of the Emscripten callback.
    /// @return EM_BOOL indicating if the event was handled.
    static EM_BOOL OnKeyUp(int eventType, const EmscriptenKeyboardEvent* keyEvent, void* inputManagerPointer);

    /// @brief Gets the current active action.
    /// @return The active action.
    ActiveAction GetAction();

    /// @brief Adds a pressed key to the deque.
    /// @param key The key string.
    void AddKeyPressed(const std::string& key) { mKeysPressed.push_front(key); }

    /// @brief Removes a released key from the deque.
    void RemoveKeyPressed(const std::string& key) { std::erase(mKeysPressed, key); }

    /// @brief Checks if any key is pressed.
    /// @return True if keys are pressed.
    int IsKeyPressed() const { return !mKeysPressed.empty(); }

private:
    /// @brief Reference to the WebInterface.
    WebInterface& mInterface;

    /// @brief Deque of currently pressed keys.
    /// We use a deque to allow for multiple keys to be pressed
    /// simultaneously and to maintain the order of key presses.
    /// Example: user presses 'w', they move up, then they press 'd',
    /// they start moving right, they release 'd', the start moving up again.
    std::deque<std::string> mKeysPressed{};
};

} // namespace cse498
