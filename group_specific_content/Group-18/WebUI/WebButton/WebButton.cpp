#include "WebButton.hpp"
#include <cassert>
#include <emscripten.h>

using emscripten::val;

// Gets the browser's document object
static val GetDocument() {
  return val::global("document");
}

// Construction / Destruction 
WebButton::WebButton(const std::string& label)
    : label_(label),
      element_(val::null()) {
  val doc = GetDocument();
  element_ = doc.call<val>("createElement", std::string("button"));
  element_.set("textContent", label_);
  element_["style"].set("position", std::string("absolute"));
  doc["body"].call<void>("appendChild", element_);
  AttachClickListener();
}

WebButton::~WebButton() {
  if (!element_.isNull()) {
    element_["parentNode"].call<void>("removeChild", element_);
    element_ = val::null();
  }
}

WebButton::WebButton(WebButton&& other) noexcept
    : label_(std::move(other.label_)),
      callback_(std::move(other.callback_)),
      is_enabled_(other.is_enabled_),
      is_visible_(other.is_visible_),
      element_(other.element_) {
  other.element_ = val::null();
  other.is_enabled_ = false;
  other.is_visible_ = false;
}

WebButton& WebButton::operator=(WebButton&& other) noexcept {
  if (this != &other) {
    if (!element_.isNull()) {
      element_["parentNode"].call<void>("removeChild", element_);
    }
    label_ = std::move(other.label_);
    callback_ = std::move(other.callback_);
    is_enabled_ = other.is_enabled_;
    is_visible_ = other.is_visible_;
    element_ = other.element_;
    other.element_ = val::null();
    other.is_enabled_ = false;
    other.is_visible_ = false;
  }
  return *this;
}

// Label

void WebButton::SetLabel(const std::string& text) {
  label_ = text;
  if (!element_.isNull()) {
    element_.set("textContent", label_);
  }
}

std::string WebButton::GetLabel() const {
  return label_;
}

// Callback

void WebButton::SetCallback(std::function<void()> callback) {
  assert(callback && "SetCallback: callback must not be null");
  callback_ = std::move(callback);
}

void WebButton::Click() {
  if (is_enabled_ && callback_) {
    callback_();
  }
}

// Size & Position

void WebButton::SetSize(int width, int height) {
  assert(width > 0 && "SetSize: width must be positive");
  assert(height > 0 && "SetSize: height must be positive");
  if (!element_.isNull()) {
    element_["style"].set("width", std::to_string(width) + "px");
    element_["style"].set("height", std::to_string(height) + "px");
  }
}

void WebButton::SetPosition(int x, int y) {
  if (!element_.isNull()) {
    element_["style"].set("left", std::to_string(x) + "px");
    element_["style"].set("top", std::to_string(y) + "px");
  }
}

// Styling

void WebButton::SetBackgroundColor(const std::string& color) {
  if (!element_.isNull()) {
    element_["style"].set("backgroundColor", color);
  }
}

void WebButton::SetTextColor(const std::string& color) {
  if (!element_.isNull()) {
    element_["style"].set("color", color);
  }
}

// State

void WebButton::Enable() {
  is_enabled_ = true;
  if (!element_.isNull()) {
    element_.set("disabled", false);
  }
}

void WebButton::Disable() {
  is_enabled_ = false;
  if (!element_.isNull()) {
    element_.set("disabled", true);
  }
}

bool WebButton::IsEnabled() const {
  return is_enabled_;
}

void WebButton::Show() {
  is_visible_ = true;
  if (!element_.isNull()) {
    element_["style"].set("display", std::string(""));
  }
}

void WebButton::Hide() {
  is_visible_ = false;
  if (!element_.isNull()) {
    element_["style"].set("display", std::string("none"));
  }
}

bool WebButton::IsVisible() const {
  return is_visible_;
}

// Private Helpers

void WebButton::AttachClickListener() {
  if (element_.isNull()) return;

  // Store a pointer to this instance for the JS callback
  WebButton* self = this;
  val callback = val::module_property("WebButton_onClick");

  // Register a global C++ function that JS can call
  EM_ASM({
    var el = Emval.toValue($0);
    var ptrVal = $1;
    el.addEventListener("click", function() {
      Module._WebButton_handleClick(ptrVal);
    });
  }, element_.as_handle(), reinterpret_cast<int>(self));
}

void WebButton::HandleClick() {
  if (is_enabled_ && callback_) {
    callback_();
  }
}

// C function that JS calls when button is clicked
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void WebButton_handleClick(int ptr) {
    WebButton* btn = reinterpret_cast<WebButton*>(ptr);
    if (btn) {
      btn->Click();
    }
  }
}