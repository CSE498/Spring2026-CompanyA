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

#endif