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

#ifndef WEBBUTTON_HPP_
#define WEBBUTTON_HPP_

#include <string>
#include <functional>
#include <emscripten/val.h>

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
class WebButton : public IDomElement {
 public:
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

  /// @brief Removes this button from its current DOM parent.
  void Unmount() override;

  /// @brief Synchronizes the DOM element with the current model state.
  void SyncFromModel() override;

  /// @brief Returns the unique DOM id assigned to this button's element.
  /// @return Const reference to the id string.
  const std::string& Id() const override;

  /// @brief Called by the JS click listener to fire the click callback.
  void HandleClick();

 private:
  std::string mLabel;                   ///< Current button label text.
  std::function<void()> mCallback;      ///< Click callback; may be empty.
  bool mIsEnabled = true;               ///< Whether the button responds to clicks.
  bool mIsVisible = true;               ///< Whether the button is visible.
  int mWidth = 0;                       ///< Width in pixels (0 = browser default).
  int mHeight = 0;                      ///< Height in pixels (0 = browser default).
  std::string mBgColor;                 ///< Background color CSS string.
  std::string mTextColor;               ///< Text color CSS string.
  emscripten::val mElement;             ///< Underlying DOM \<button\> element.
  std::string mId;                      ///< Unique DOM id for this button.

  static int mNextIdCounter;            ///< Counter for generating unique DOM ids.

  /// @brief Attaches the JS click event listener to the DOM element.
  void AttachClickListener();
};
}

#endif