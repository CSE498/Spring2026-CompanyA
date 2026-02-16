#include "WebButton.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <cassert>
#include <emscripten.h>

using emscripten::val;

int WebButton::next_id_counter_ = 1;

static val GetDocument() {
  return val::global("document");
}

static std::string ToPx(int value) {
  return std::to_string(value) + "px";
}

WebButton::WebButton(const std::string& label)
    : label_(label),
      element_(val::null()) {
  id_ = "webbutton-" + std::to_string(next_id_counter_++);

  val doc = GetDocument();
  element_ = doc.call<val>("createElement", std::string("button"));
  element_.set("id", id_);
  element_.set("textContent", label_);
  element_["style"].set("boxSizing", std::string("border-box"));

  doc["body"].call<void>("appendChild", element_);
  AttachClickListener();
}

WebButton::~WebButton() {
  unmount();
  element_ = val::null();
}

WebButton::WebButton(WebButton&& other) noexcept
    : label_(std::move(other.label_)),
      callback_(std::move(other.callback_)),
      is_enabled_(other.is_enabled_),
      is_visible_(other.is_visible_),
      width_(other.width_),
      height_(other.height_),
      bg_color_(std::move(other.bg_color_)),
      text_color_(std::move(other.text_color_)),
      element_(other.element_),
      id_(std::move(other.id_)) {
  other.element_ = val::null();
  other.is_enabled_ = false;
  other.is_visible_ = false;
  other.width_ = 0;
  other.height_ = 0;
}

WebButton& WebButton::operator=(WebButton&& other) noexcept {
  if (this != &other) {
    unmount();

    label_ = std::move(other.label_);
    callback_ = std::move(other.callback_);
    is_enabled_ = other.is_enabled_;
    is_visible_ = other.is_visible_;
    width_ = other.width_;
    height_ = other.height_;
    bg_color_ = std::move(other.bg_color_);
    text_color_ = std::move(other.text_color_);
    element_ = other.element_;
    id_ = std::move(other.id_);

    other.element_ = val::null();
    other.is_enabled_ = false;
    other.is_visible_ = false;
    other.width_ = 0;
    other.height_ = 0;
  }
  return *this;
}

void WebButton::SetLabel(const std::string& text) {
  label_ = text;
  if (!element_.isNull()) {
    element_.set("textContent", label_);
  }
}

std::string WebButton::GetLabel() const {
  return label_;
}

void WebButton::SetCallback(std::function<void()> callback) {
  assert(callback && "SetCallback: callback must not be null");
  callback_ = std::move(callback);
}

void WebButton::Click() {
  if (is_enabled_ && callback_) {
    callback_();
  }
}

void WebButton::SetSize(int width, int height) {
  assert(width >= 0 && "SetSize: width must be non-negative");
  assert(height >= 0 && "SetSize: height must be non-negative");
  width_ = width;
  height_ = height;
  if (!element_.isNull()) {
    if (width > 0) {
      element_["style"].set("width", ToPx(width));
    }
    if (height > 0) {
      element_["style"].set("height", ToPx(height));
    }
  }
}

int WebButton::GetWidth() const {
  return width_;
}

int WebButton::GetHeight() const {
  return height_;
}

void WebButton::SetBackgroundColor(const std::string& color) {
  bg_color_ = color;
  if (!element_.isNull()) {
    element_["style"].set("backgroundColor", color);
  }
}

void WebButton::SetTextColor(const std::string& color) {
  text_color_ = color;
  if (!element_.isNull()) {
    element_["style"].set("color", color);
  }
}

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

void WebButton::mountToLayout(WebLayout& parent, Alignment align) {
  parent.AddElement(id_, align);
}

void WebButton::unmount() {
  if (element_.isNull()) return;

  val parent = element_["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", element_);
  }
}

void WebButton::syncFromModel() {
  if (element_.isNull()) return;

  element_.set("textContent", label_);
  element_.set("disabled", !is_enabled_);

  if (width_ > 0) {
    element_["style"].set("width", ToPx(width_));
  }
  if (height_ > 0) {
    element_["style"].set("height", ToPx(height_));
  }

  if (!bg_color_.empty()) {
    element_["style"].set("backgroundColor", bg_color_);
  }
  if (!text_color_.empty()) {
    element_["style"].set("color", text_color_);
  }

  element_["style"].set("display",
      std::string(is_visible_ ? "" : "none"));
}

const std::string& WebButton::Id() const {
  return id_;
}

void WebButton::HandleClick() {
  Click();
}

void WebButton::AttachClickListener() {
  if (element_.isNull()) return;

  EM_ASM({
    var el = Emval.toValue($0);
    var ptrVal = $1;
    el.addEventListener("click", function() {
      Module._WebButton_handleClick(ptrVal);
    });
  }, element_.as_handle(), reinterpret_cast<intptr_t>(this));
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