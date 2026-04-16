/**
 * @file WebButton.hpp
 * @brief Declaration of WebButton, a DOM-backed clickable button for WebUI.
 *
 * WebButton wraps an HTML \<button\> element and exposes a C++ API for
 * setting its label, click callback, dimensions, colors, enabled/disabled
 * state, and visibility. It implements IDomElement so it can be mounted
 * into WebLayout containers.
 *
 */

#pragma once

#include <emscripten/val.h>
#include <functional>
#include <string>

#include "../internal/ICanvasElement.hpp"
#include "../internal/IDomElement.hpp"

namespace cse498 {

    /**
     * @class WebButton
     * @brief DOM-backed clickable button element for use in WebUI layouts.
     *
     * WebButton wraps an HTML \<button\> element and provides a C++ API for
     * configuring its label, click callback, size, colors, and visibility.
     * It implements IDomElement so it can be mounted into and managed by
     * WebLayout containers.
     */
    class WebButton : public IDomElement, public ICanvasElement {
    public:
        static constexpr int kDefaultSize = 0;
        static constexpr float kDefaultCanvasWidth = 80.0f;
        static constexpr float kDefaultCanvasHeight = 30.0f;
        static constexpr float kDefaultFontSize = 14.0f;
        static constexpr float kLabelVerticalCenter = 0.7f;
        static constexpr float kLabelHorizontalFactor = 0.3f;
        static constexpr const char* kIdPrefix = "webbutton-";

        /// @brief Creates a button with the given label, appended to the document body.
        /// @param label Display text for the button.
        explicit WebButton(const std::string& label = "");

        ~WebButton();

        WebButton(const WebButton&) = delete;
        WebButton& operator=(const WebButton&) = delete;

        /// @brief Move constructor; transfers DOM ownership from @p other.
        /// @param other Source WebButton to move from.
        WebButton(WebButton&& other) noexcept;

        /// @brief Move assignment; transfers DOM ownership from @p other.
        /// @param other Source WebButton to move from.
        /// @return Reference to this object.
        WebButton& operator=(WebButton&& other) noexcept;

        /// @brief Sets the button display text.
        /// @param text New label string.
        void SetLabel(const std::string& text);

        /// @brief Returns the current button label.
        /// @return Copy of the current label string.
        std::string GetLabel() const;

        /// @brief Sets the click handler; callback must not be null.
        /// @param callback Callable invoked when the button is clicked while enabled.
        void SetCallback(std::function<void()> callback);

        /// @brief Sets the click handler from any callable type without
        ///        std::function overhead. The callable must be invocable
        ///        with no arguments and return void.
        /// @tparam F Callable type (lambda, functor, function pointer).
        /// @param callable The callable to store as the click handler.
        template<typename F>
            requires std::invocable<F>
        void SetCallback(F&& callable) {
            mCallback = std::function<void()>(std::forward<F>(callable));
        }

        /// @brief Fires the click callback if the button is enabled and a callback is set.
        void Click();

        /// @brief Sets button dimensions in pixels; 0 means use the browser default.
        /// @param width  Desired width in pixels (must be >= 0).
        /// @param height Desired height in pixels (must be >= 0).
        void SetSize(int width, int height);

        /// @brief Returns the current button width in pixels (0 = browser default).
        /// @return Width in pixels.
        int GetWidth() const;

        /// @brief Returns the current button height in pixels (0 = browser default).
        /// @return Height in pixels.
        int GetHeight() const;

        /// @brief Sets the background color using any valid CSS color string.
        /// @param color CSS color string.
        void SetBackgroundColor(const std::string& color);

        /// @brief Sets the text color using any valid CSS color string.
        /// @param color CSS color string.
        void SetTextColor(const std::string& color);

        /// @brief Enables the button and removes the disabled attribute from the DOM element.
        void Enable();

        /// @brief Disables the button and sets the disabled attribute on the DOM element.
        void Disable();

        /// @brief Returns whether the button is currently enabled.
        /// @return True if enabled; false if disabled.
        bool IsEnabled() const;

        /// @brief Makes the button visible by clearing the CSS display property.
        void Show();

        /// @brief Hides the button by setting display: none on the DOM element.
        void Hide();

        /// @brief Returns whether the button is currently visible.
        /// @return True if visible; false if hidden.
        bool IsVisible() const;

        /// @brief Mounts this button into the given parent layout.
        /// @param parent Parent WebLayout to attach to.
        /// @param align  Alignment within the parent container.
        void MountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;

        /// @brief Synchronizes the DOM element with the current model state.
        void SyncFromModel() override;

        /// @brief Called by the JS click listener to fire the click callback.
        void HandleClick();

        // ----- ICanvasElement Interface -----

        /// @brief Sets the position and size used when drawing this button on a WebCanvas.
        /// @param x Canvas x coordinate (left edge).
        /// @param y Canvas y coordinate (top edge).
        /// @param w Button width in pixels; pass -1 to use GetWidth() value.
        /// @param h Button height in pixels; pass -1 to use GetHeight() value.
        void SetCanvasRect(float x, float y, float w = -1.0f, float h = -1.0f);

        /// @brief Draws this button onto the given WebCanvas.
        /// Renders the button background rectangle and label text.
        /// @param canvas The WebCanvas to draw onto.
        void Draw(WebCanvas& canvas) override;

    private:
        std::string mLabel;
        std::function<void()> mCallback;
        bool mIsEnabled = true;
        bool mIsVisible = true;
        int mWidth = kDefaultSize;
        int mHeight = kDefaultSize;
        std::string mBgColor;
        std::string mTextColor;

        float mCanvasX = 0.0f;
        float mCanvasY = 0.0f;
        float mCanvasW = -1.0f;
        float mCanvasH = -1.0f;

        static int mNextIdCounter;

        void AttachClickListener();

        /// @brief Applies a CSS style property to the DOM element if it exists.
        /// @param property CSS property name.
        /// @param value CSS property value.
        void ApplyStyle(const std::string& property, const std::string& value);
    };
} // namespace cse498
