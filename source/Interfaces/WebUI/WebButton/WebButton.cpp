/**
 * @file WebButton.cpp
 * @brief Implementation of WebButton, a DOM-backed clickable button for WebUI.
 *
 * Contains the DOM interaction logic for creating a \<button\> element,
 * attaching a JavaScript click listener that forwards events back to C++,
 * and keeping the element's state synchronized with the model.
 *
 */

#include "WebButton.hpp"
#include "../WebCanvas/WebCanvas.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <cassert>
#include <emscripten.h>

using emscripten::val;

namespace cse498 {

int WebButton::mNextIdCounter = 1;

static std::string ToPx(int value) {
  return std::to_string(value) + "px";
}

WebButton::WebButton(const std::string& label)
    : mLabel(label) {
  mId = std::string(kIdPrefix) + std::to_string(mNextIdCounter++);

  val doc = GetDocument();
  mElement = doc.call<val>("createElement", std::string("button"));
  mElement.set("id", mId);
  mElement.set("textContent", mLabel);
  mElement["style"].set("boxSizing", std::string("border-box"));

  AttachClickListener();
}

WebButton::~WebButton() {
  Unmount();
  mElement = val::null();
}

WebButton::WebButton(WebButton&& other) noexcept
    : mLabel(std::move(other.mLabel)),
      mCallback(std::move(other.mCallback)),
      mIsEnabled(other.mIsEnabled),
      mIsVisible(other.mIsVisible),
      mWidth(other.mWidth),
      mHeight(other.mHeight),
      mBgColor(std::move(other.mBgColor)),
      mTextColor(std::move(other.mTextColor)) {
  mId = std::move(other.mId);
  mElement = std::move(other.mElement);
  other.mElement = val::null();
  other.mIsEnabled = false;
  other.mIsVisible = false;
  other.mWidth = kDefaultSize;
  other.mHeight = kDefaultSize;
}

WebButton& WebButton::operator=(WebButton&& other) noexcept {
  if (this != &other) {
    Unmount();

    mLabel = std::move(other.mLabel);
    mCallback = std::move(other.mCallback);
    mIsEnabled = other.mIsEnabled;
    mIsVisible = other.mIsVisible;
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mBgColor = std::move(other.mBgColor);
    mTextColor = std::move(other.mTextColor);
    mElement = other.mElement;
    mId = std::move(other.mId);

    other.mElement = val::null();
    other.mIsEnabled = false;
    other.mIsVisible = false;
    other.mWidth = kDefaultSize;
    other.mHeight = kDefaultSize;
  }
  return *this;
}

void WebButton::SetLabel(const std::string& text) {
  mLabel = text;
  if (!mElement.isNull()) {
    mElement.set("textContent", mLabel);
  }
}

std::string WebButton::GetLabel() const {
  return mLabel;
}

void WebButton::SetCallback(std::function<void()> callback) {
  assert(callback && "SetCallback: callback must not be null");
  mCallback = std::move(callback);
}

void WebButton::Click() {
  if (mIsEnabled && mCallback) {
    mCallback();
  }
}

void WebButton::SetSize(int width, int height) {
  assert(width >= 0 && "SetSize: width must be non-negative");
  assert(height >= 0 && "SetSize: height must be non-negative");
  mWidth = width;
  mHeight = height;
  ApplyStyle("width", width > 0 ? ToPx(width) : "auto");
  ApplyStyle("height", height > 0 ? ToPx(height) : "auto");
}

int WebButton::GetWidth() const {
  return mWidth;
}

int WebButton::GetHeight() const {
  return mHeight;
}

void WebButton::SetBackgroundColor(const std::string& color) {
  mBgColor = color;
  ApplyStyle("backgroundColor", color);
}

void WebButton::SetTextColor(const std::string& color) {
  mTextColor = color;
  ApplyStyle("color", color);
}

void WebButton::Enable() {
  mIsEnabled = true;
  if (!mElement.isNull()) {
    mElement.set("disabled", false);
  }
}

void WebButton::Disable() {
  mIsEnabled = false;
  if (!mElement.isNull()) {
    mElement.set("disabled", true);
  }
}

bool WebButton::IsEnabled() const {
  return mIsEnabled;
}

void WebButton::Show() {
  mIsVisible = true;
  ApplyStyle("display", "");
}

void WebButton::Hide() {
  mIsVisible = false;
  ApplyStyle("display", "none");
}

bool WebButton::IsVisible() const {
  return mIsVisible;
}

void WebButton::MountToLayout(WebLayout& parent, Alignment align) {
  parent.AddElement(this, align);
}

void WebButton::SyncFromModel() {
  if (mElement.isNull()) return;

  mElement.set("textContent", mLabel);
  mElement.set("disabled", !mIsEnabled);

  ApplyStyle("width", mWidth > 0 ? ToPx(mWidth) : "auto");
  ApplyStyle("height", mHeight > 0 ? ToPx(mHeight) : "auto");

  if (!mBgColor.empty()) {
    ApplyStyle("backgroundColor", mBgColor);
  }
  if (!mTextColor.empty()) {
    ApplyStyle("color", mTextColor);
  }

  ApplyStyle("display", mIsVisible ? "" : "none");
}

void WebButton::HandleClick() {
  Click();
}

void WebButton::AttachClickListener() {
  if (mElement.isNull()) return;

  EM_ASM({
    var el = Emval.toValue($0);
    var ptrVal = $1;
    el.addEventListener("click", function() {
      Module._WebButton_handleClick(ptrVal);
    });
  }, mElement.as_handle(), reinterpret_cast<intptr_t>(this));
}

void WebButton::ApplyStyle(const std::string& property, const std::string& value) {
  if (!mElement.isNull()) {
    mElement["style"].set(property, value);
  }
}

// ----- ICanvasElement Interface -----

void WebButton::SetCanvasRect(float x, float y, float w, float h) {
  mCanvasX = x;
  mCanvasY = y;
  mCanvasW = w;
  mCanvasH = h;
}

void WebButton::Draw(WebCanvas& canvas) {
  float w = (mCanvasW > 0) ? mCanvasW
      : static_cast<float>(mWidth > 0 ? mWidth : kDefaultCanvasWidth);
  float h = (mCanvasH > 0) ? mCanvasH
      : static_cast<float>(mHeight > 0 ? mHeight : kDefaultCanvasHeight);

  std::string bg = mIsEnabled
      ? (mBgColor.empty() ? std::string(kDefaultBgColor) : mBgColor)
      : std::string(kDisabledBgColor);

  canvas.DrawRect(mCanvasX, mCanvasY, w, h, bg);

  if (!mLabel.empty()) {
    std::string textCol = mTextColor.empty()
        ? std::string(kDefaultTextColor) : mTextColor;
    float tx = mCanvasX + w * 0.5f
        - static_cast<float>(mLabel.size()) * kDefaultFontSize * kLabelHorizontalFactor;
    float ty = mCanvasY + h * kLabelVerticalCenter;
    canvas.DrawText(tx, ty, mLabel, textCol, kDefaultFontSize, "");
  }
}

}

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void WebButton_handleClick(intptr_t ptr) {
    auto* btn = reinterpret_cast<cse498::WebButton*>(ptr);
    if (btn) {
      btn->HandleClick();
    }
  }
}