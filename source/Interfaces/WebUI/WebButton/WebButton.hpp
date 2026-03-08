#ifndef WEBBUTTON_HPP_
#define WEBBUTTON_HPP_

#include <string>
#include <functional>
#include <emscripten/val.h>

#include "../internal/IDomElement.hpp"

namespace cse498 {

class WebButton : public IDomElement {
 public:
  /// Creates a button with the given label, appended to document body
  explicit WebButton(const std::string& label = "");
  ~WebButton();

  WebButton(const WebButton&) = delete;
  WebButton& operator=(const WebButton&) = delete;

  WebButton(WebButton&& other) noexcept;
  WebButton& operator=(WebButton&& other) noexcept;

  /// Sets the button display text
  void SetLabel(const std::string& text);
  std::string GetLabel() const;

  /// Sets the click handler; callback must not be null
  void SetCallback(std::function<void()> callback);
  /// Fires the callback if enabled and callback is set
  void Click();

  /// Sets button dimensions in pixels; 0 means use browser default
  void SetSize(int width, int height);
  int GetWidth() const;
  int GetHeight() const;

  /// Sets background color using any valid CSS color string
  void SetBackgroundColor(const std::string& color);
  /// Sets text color using any valid CSS color string
  void SetTextColor(const std::string& color);

  void Enable();
  void Disable();
  bool IsEnabled() const;

  void Show();
  void Hide();
  bool IsVisible() const;

  void MountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
  void Unmount() override;
  void SyncFromModel() override;
  const std::string& Id() const override;

  void HandleClick();

 private:
  std::string mLabel;
  std::function<void()> mCallback;
  bool mIsEnabled = true;
  bool mIsVisible = true;
  int mWidth = 0;
  int mHeight = 0;
  std::string mBgColor;
  std::string mTextColor;
  emscripten::val mElement;
  std::string mId;

  static int mNextIdCounter;

  void AttachClickListener();
};
}

#endif