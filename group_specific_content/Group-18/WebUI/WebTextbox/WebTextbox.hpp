#ifndef WEBTEXTBOX_HPP_
#define WEBTEXTBOX_HPP_

#include <string>
#include <emscripten/val.h>

/// Manages an HTML <input type="text"> element from C++ via Emscripten
class WebTextbox {
public:
  // Construction / Destruction
  explicit WebTextbox(const std::string& initial_text = "");
  ~WebTextbox();

  // Each instance owns a unique DOM element
  WebTextbox(const WebTextbox&) = delete;
  WebTextbox& operator=(const WebTextbox&) = delete;

  // Transfers DOM ownership
  WebTextbox(WebTextbox&& other) noexcept;
  WebTextbox& operator=(WebTextbox&& other) noexcept;

  // Core Text Operations
  void SetText(const std::string& text);
  void AppendText(const std::string& text);
  std::string GetText() const;
  void Clear();

  // Formatting
  void SetFontFamily(const std::string& family);
  void SetFontSize(float size_px);
  void SetColor(const std::string& css_color);
  void SetBold(bool enabled);
  void SetItalic(bool enabled);
  void SetAlignment(const std::string& alignment);  // "left", "center", "right"
  void SetMaxWidth(float width_px);

  // Layout
  void SetPosition(int x, int y);

  // Visibility
  void Show();
  void Hide();
  bool IsVisible() const;

private:
  std::string text_;
  bool is_visible_ = true;
  emscripten::val element_;  // DOM element

  void ApplyText();
};

#endif  // WEBTEXTBOX_HPP_