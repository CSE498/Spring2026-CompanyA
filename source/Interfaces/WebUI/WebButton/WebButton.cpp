#include "WebButton.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <cassert>
#include <emscripten.h>

using emscripten::val;

int WebButton::mNextIdCounter = 1;

static val GetDocument() {
  return val::global("document");
}

static std::string ToPx(int value) {
  return std::to_string(value) + "px";
}

WebButton::WebButton(const std::string& label)
    : mLabel(label),
      mElement(val::null()) {
  mId = "webbutton-" + std::to_string(mNextIdCounter++);

  val doc = GetDocument();
  mElement = doc.call<val>("createElement", std::string("button"));
  mElement.set("id", mId);
  mElement.set("textContent", mLabel);
  mElement["style"].set("boxSizing", std::string("border-box"));

  doc["body"].call<void>("appendChild", mElement);
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
      mTextColor(std::move(other.mTextColor)),
      mElement(other.mElement),
      mId(std::move(other.mId)) {
  other.mElement = val::null();
  other.mIsEnabled = false;
  other.mIsVisible = false;
  other.mWidth = 0;
  other.mHeight = 0;
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
    other.mWidth = 0;
    other.mHeight = 0;
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
  if (!mElement.isNull()) {
    mElement["style"].set("width", width > 0 ? ToPx(width) : std::string("auto"));
    mElement["style"].set("height", height > 0 ? ToPx(height) : std::string("auto"));
  }
}

int WebButton::GetWidth() const {
  return mWidth;
}

int WebButton::GetHeight() const {
  return mHeight;
}

void WebButton::SetBackgroundColor(const std::string& color) {
  mBgColor = color;
  if (!mElement.isNull()) {
    mElement["style"].set("backgroundColor", color);
  }
}

void WebButton::SetTextColor(const std::string& color) {
  mTextColor = color;
  if (!mElement.isNull()) {
    mElement["style"].set("color", color);
  }
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
  if (!mElement.isNull()) {
    mElement["style"].set("display", std::string(""));
  }
}

void WebButton::Hide() {
  mIsVisible = false;
  if (!mElement.isNull()) {
    mElement["style"].set("display", std::string("none"));
  }
}

bool WebButton::IsVisible() const {
  return mIsVisible;
}

void WebButton::MountToLayout(WebLayout& parent, Alignment align) {
  parent.AddElement(this, align);
}

void WebButton::Unmount() {
  if (mElement.isNull()) return;

  val parent = mElement["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", mElement);
  }
}

void WebButton::SyncFromModel() {
  if (mElement.isNull()) return;

  mElement.set("textContent", mLabel);
  mElement.set("disabled", !mIsEnabled);

  mElement["style"].set("width", mWidth > 0 ? ToPx(mWidth) : std::string("auto"));
  mElement["style"].set("height", mHeight > 0 ? ToPx(mHeight) : std::string("auto"));

  if (!mBgColor.empty()) {
    mElement["style"].set("backgroundColor", mBgColor);
  }
  if (!mTextColor.empty()) {
    mElement["style"].set("color", mTextColor);
  }

  mElement["style"].set("display",
      std::string(mIsVisible ? "" : "none"));
}

const std::string& WebButton::Id() const {
  return mId;
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

extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void WebButton_handleClick(intptr_t ptr) {
    WebButton* btn = reinterpret_cast<WebButton*>(ptr);
    if (btn) {
      btn->HandleClick();
    }
  }
}