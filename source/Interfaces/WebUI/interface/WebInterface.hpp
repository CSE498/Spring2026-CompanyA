#pragma once

#include "../../../core/InterfaceBase.hpp"
#include "../InputManager.hpp"
#include "../internal/IDomElement.hpp"

#include <emscripten/val.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace cse498 {

class WebLayout;
class WebTextbox;
class WebCanvas;

/// @class WebInterface
/// @brief Web-based interface implementation for the game.
/// @details This class provides a web UI using Emscripten, managing layouts, canvas, input, and rendering.
class WebInterface : public InterfaceBase {

public:
    /// @brief Constructs a WebInterface.
    /// @param id The interface ID.
    /// @param name The interface name.
    /// @param world Reference to the world.
    WebInterface(size_t id, const std::string& name, const WorldBase& world);
    ~WebInterface() = default;

    /// @brief Selects an action based on user input.
    /// @param grid The world grid.
    /// @return The selected action ID.
    [[nodiscard]] size_t SelectAction(const WorldGrid& grid) override;

    /// @brief Runs after ConfigAgent to ensure configuration was successful
    /// @return true for success, false for failure
    bool Initialize() override;

    /// @brief Renders the current frame.
    void RenderFrame() override;

    /// @brief Handles pause toggle.
    void HandlePause();

private:
    /// @brief Input manager for handling user input.
    InputManager mInputManager{*this};

    /// @brief Vector of DOM elements managed by the interface.
    std::vector<std::unique_ptr<IDomElement>> mElements{};

    /// @brief Current points score.
    int mPoints{0};

    /// @brief Description of the world.
    std::string mWorldDescription{};

    /// @brief Root layout element.
    WebLayout* mRoot = nullptr;

    /// @brief Pause menu layout.
    WebLayout* mPauseMenu = nullptr;

    /// @brief Canvas for rendering the game grid.
    WebCanvas* mCanvas = nullptr;

    /// @brief Textbox displaying points.
    WebTextbox* mPointsTextbox = nullptr;

    /// @brief Map of textures for different symbols.
    std::unordered_map<char, emscripten::val> mTextures{};


    /// @brief Draws the world grid on the canvas.
    /// @param grid The world grid to draw.
    /// @param itemIds IDs of items to display.
    /// @param agentIds IDs of agents to display.
    void DrawGrid(const WorldGrid& grid, const std::vector<size_t>& itemIds, const std::vector<size_t>& agentIds);
};

} // namespace cse498
