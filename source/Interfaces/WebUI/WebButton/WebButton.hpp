#ifndef WEBBUTTON_HPP_
#define WEBBUTTON_HPP_

#include <string>
#include <functional>
#include <emscripten/val.h>

#include "../internal/IDomElement.hpp"

class WebButton : public IDomElement {
 public:
  explicit WebButton(const std::string& label = "");
  ~WebButton();

  WebButton(const WebButton&) = delete;
  WebButton& operator=(const WebButton&) = delete;

  WebButton(WebButton&& other) noexcept;
  WebButton& operator=(WebButton&& other) noexcept;

  void SetLabel(const std::string& text);
  std::string GetLabel() const;

  void SetCallback(std::function<void()> callback);
  void Click();

  void SetSize(int width, int height);
  int GetWidth() const;
  int GetHeight() const;

  void SetBackgroundColor(const std::string& color);
  void SetTextColor(const std::string& color);

  void Enable();
  void Disable();
  bool IsEnabled() const;

  void Show();
  void Hide();
  bool IsVisible() const;

  void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
  void unmount() override;
  void syncFromModel() override;
  const std::string& Id() const override;

  void HandleClick();

 private:
  std::string label_;
  std::function<void()> callback_;
  bool is_enabled_ = true;
  bool is_visible_ = true;
  int width_ = 0;
  int height_ = 0;
  std::string bg_color_;
  std::string text_color_;
  emscripten::val element_;
  std::string id_;

  static int next_id_counter_;

  void AttachClickListener();
};

#endif