#ifndef WEBBUTTON_HPP_
#define WEBBUTTON_HPP_

#include <string>
#include <functional>
#include <emscripten/val.h>

/// Manages an HTML <button> element from C++ via Emscripten
class WebButton {
 public:
  explicit WebButton(const std::string& label);
  ~WebButton();

  // Each instance owns a unique DOM element
  WebButton(const WebButton&) = delete;
  WebButton& operator=(const WebButton&) = delete;

  // Transfers DOM ownership
  WebButton(WebButton&& other) noexcept;
  WebButton& operator=(WebButton&& other) noexcept;

  // Label
  void SetLabel(const std::string& text);
  std::string GetLabel() const;

  // Callback
  void SetCallback(std::function<void()> callback);
  void Click();

  // Size and position
  void SetSize(int width, int height);
  void SetPosition(int x, int y);

  // Styling (any valid CSS color string)
  void SetBackgroundColor(const std::string& color);
  void SetTextColor(const std::string& color);

  // State
  void Enable();
  void Disable();
  bool IsEnabled() const;
  void Show();
  void Hide();
  bool IsVisible() const;

 private:
  std::string label_;
  std::function<void()> callback_;
  bool is_enabled_ = true;
  bool is_visible_ = true;
  emscripten::val element_;

  void AttachClickListener();
  void HandleClick();
};

#endif 