#include "WebImage.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <cassert>
#include <iostream>
#include <emscripten.h>

using emscripten::val;

int WebImage::next_id_counter_ = 1;

// Helper: Get the browser's document object
static val GetDocument() {
  return val::global("document");
}

// Helper: Convert int to CSS pixel string
static std::string ToPx(int value) {
  return std::to_string(value) + "px";
}

// ----- Construction / Destruction -----

WebImage::WebImage(const std::string& src, const std::string& alt_text)
    : src_(src),
      alt_text_(alt_text),
      element_(val::null()) {
  // Generate a unique ID for this element
  id_ = "webimage-" + std::to_string(next_id_counter_++);

  val doc = GetDocument();
  element_ = doc.call<val>("createElement", std::string("img"));
  element_.set("id", id_);
  element_.set("src", src_);
  element_.set("alt", alt_text_);
  doc["body"].call<void>("appendChild", element_);

  AttachListeners();
}

WebImage::~WebImage() {
  if (!element_.isNull()) {
    val parent = element_["parentNode"];
    if (!parent.isNull() && !parent.isUndefined()) {
      parent.call<void>("removeChild", element_);
    }
    element_ = val::null();
  }
}

WebImage::WebImage(WebImage&& other) noexcept
    : src_(std::move(other.src_)),
      alt_text_(std::move(other.alt_text_)),
      width_(other.width_),
      height_(other.height_),
      opacity_(other.opacity_),
      is_visible_(other.is_visible_),
      is_loaded_(other.is_loaded_),
      has_error_(other.has_error_),
      error_mode_(other.error_mode_),
      placeholder_color_(std::move(other.placeholder_color_)),
      on_load_callback_(std::move(other.on_load_callback_)),
      on_error_callback_(std::move(other.on_error_callback_)),
      element_(other.element_),
      id_(std::move(other.id_)) {
  other.element_ = val::null();
  other.width_ = 0;
  other.height_ = 0;
  other.opacity_ = 1.0;
  other.is_visible_ = false;
  other.is_loaded_ = false;
  other.has_error_ = false;
}

WebImage& WebImage::operator=(WebImage&& other) noexcept {
  if (this != &other) {
    // Clean up current element
    if (!element_.isNull()) {
      val parent = element_["parentNode"];
      if (!parent.isNull() && !parent.isUndefined()) {
        parent.call<void>("removeChild", element_);
      }
    }

    src_ = std::move(other.src_);
    alt_text_ = std::move(other.alt_text_);
    width_ = other.width_;
    height_ = other.height_;
    opacity_ = other.opacity_;
    is_visible_ = other.is_visible_;
    is_loaded_ = other.is_loaded_;
    has_error_ = other.has_error_;
    error_mode_ = other.error_mode_;
    placeholder_color_ = std::move(other.placeholder_color_);
    on_load_callback_ = std::move(other.on_load_callback_);
    on_error_callback_ = std::move(other.on_error_callback_);
    element_ = other.element_;
    id_ = std::move(other.id_);

    other.element_ = val::null();
    other.width_ = 0;
    other.height_ = 0;
    other.opacity_ = 1.0;
    other.is_visible_ = false;
    other.is_loaded_ = false;
    other.has_error_ = false;
  }
  return *this;
}

// ----- Source & Metadata -----

void WebImage::SetSource(const std::string& src) {
  src_ = src;
  is_loaded_ = false;
  has_error_ = false;
  if (!element_.isNull()) {
    // Reset to <img> in case we had replaced it with a placeholder <div>
    element_.set("src", src_);
    element_["style"].set("backgroundColor", std::string(""));
  }
}

std::string WebImage::GetSource() const {
  return src_;
}

void WebImage::SetAltText(const std::string& alt_text) {
  alt_text_ = alt_text;
  if (!element_.isNull()) {
    element_.set("alt", alt_text_);
  }
}

std::string WebImage::GetAltText() const {
  return alt_text_;
}

// ----- Sizing -----

void WebImage::SetSize(int width_px, int height_px) {
  assert(width_px >= 0 && "SetSize: width must be non-negative");
  assert(height_px >= 0 && "SetSize: height must be non-negative");
  width_ = width_px;
  height_ = height_px;
  if (!element_.isNull()) {
    if (width_px > 0) {
      element_["style"].set("width", ToPx(width_px));
    }
    if (height_px > 0) {
      element_["style"].set("height", ToPx(height_px));
    }
    // Stretch to exact dimensions (no aspect ratio preservation)
    element_["style"].set("objectFit", std::string("fill"));
  }
}

void WebImage::Resize(int width_px, int height_px, bool maintain_aspect_ratio) {
  assert(width_px > 0 && "Resize: width must be positive");
  assert(height_px > 0 && "Resize: height must be positive");
  width_ = width_px;
  height_ = height_px;
  if (!element_.isNull()) {
    element_["style"].set("width", ToPx(width_px));
    element_["style"].set("height", ToPx(height_px));
    // "contain" scales to fit within the box keeping aspect ratio
    // "fill" stretches to fill exact dimensions
    element_["style"].set("objectFit",
        std::string(maintain_aspect_ratio ? "contain" : "fill"));
  }
}

int WebImage::GetWidth() const {
  return width_;
}

int WebImage::GetHeight() const {
  return height_;
}

// ----- Opacity / Transparency -----

void WebImage::SetOpacity(double alpha) {
  assert(alpha >= 0.0 && alpha <= 1.0 && "SetOpacity: alpha must be in [0.0, 1.0]");
  opacity_ = alpha;
  if (!element_.isNull()) {
    element_["style"].set("opacity", std::to_string(alpha));
  }
}

double WebImage::GetOpacity() const {
  return opacity_;
}

// ----- Visibility -----

void WebImage::Show() {
  is_visible_ = true;
  if (!element_.isNull()) {
    element_["style"].set("display", std::string(""));
  }
}

void WebImage::Hide() {
  is_visible_ = false;
  if (!element_.isNull()) {
    element_["style"].set("display", std::string("none"));
  }
}

bool WebImage::IsVisible() const {
  return is_visible_;
}

// ----- Loading State & Error Handling -----

void WebImage::MarkLoaded(bool loaded) {
  is_loaded_ = loaded;
}

bool WebImage::IsLoaded() const {
  return is_loaded_;
}

bool WebImage::HasError() const {
  return has_error_;
}

void WebImage::SetOnLoadCallback(std::function<void()> callback) {
  on_load_callback_ = std::move(callback);
}

void WebImage::SetOnErrorCallback(std::function<void()> callback) {
  on_error_callback_ = std::move(callback);
}

void WebImage::SetErrorMode(ImageErrorMode mode) {
  error_mode_ = mode;
}

void WebImage::SetPlaceholderColor(const std::string& css_color) {
  placeholder_color_ = css_color;
}

// ----- IDomElement Interface -----

void WebImage::mountToLayout(WebLayout& parent, Alignment align) {
  parent.AddElement(this, align);
}

void WebImage::unmount() {
  val parent = element_["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", element_);
  }
}

void WebImage::syncFromModel() {
  // Re-apply all properties to DOM element
  if (element_.isNull()) return;

  element_.set("src", src_);
  element_.set("alt", alt_text_);

  if (width_ > 0) {
    element_["style"].set("width", ToPx(width_));
  }
  if (height_ > 0) {
    element_["style"].set("height", ToPx(height_));
  }

  element_["style"].set("opacity", std::to_string(opacity_));
  element_["style"].set("display",
      std::string(is_visible_ ? "" : "none"));
}

const std::string& WebImage::Id() const {
  return id_;
}

// ----- ICanvasElement Interface -----

void WebImage::draw(WebCanvas& canvas) {
  // Canvas drawing implementation
  // This is a stub - actual implementation would use canvas 2D context
  // to draw the image at the specified position and size.
  // For now, WebImage primarily supports DOM-based rendering.
  (void)canvas;  // Suppress unused parameter warning
}

// ----- Event Handlers -----

void WebImage::HandleLoad() {
  is_loaded_ = true;
  has_error_ = false;
  if (on_load_callback_) {
    on_load_callback_();
  }
}

void WebImage::HandleError() {
  has_error_ = true;
  is_loaded_ = false;
  std::cerr << "WebImage error: failed to load image '" << src_
            << "' (id=" << id_ << ")" << std::endl;

  if (error_mode_ == ImageErrorMode::BlankRect) {
    ApplyPlaceholder();
  }
  // NoOp: do nothing, leave the broken image as-is

  if (on_error_callback_) {
    on_error_callback_();
  }
}

// ----- Private Helpers -----

void WebImage::AttachListeners() {
  if (element_.isNull()) return;

  WebImage* self = this;

  EM_ASM({
    var el = Emval.toValue($0);
    var ptrVal = $1;
    el.addEventListener("load", function() {
      Module._WebImage_handleLoad(ptrVal);
    });
    el.addEventListener("error", function() {
      Module._WebImage_handleError(ptrVal);
    });
  }, element_.as_handle(), reinterpret_cast<intptr_t>(self));
}

void WebImage::ApplyPlaceholder() {
  if (element_.isNull()) return;

  // Hide the broken image icon by removing the src
  element_.set("src", std::string(""));

  // Show a colored rectangle instead
  val style = element_["style"];
  style.set("backgroundColor", placeholder_color_);
  style.set("display", std::string("inline-block"));

  // Ensure the placeholder has visible dimensions
  if (width_ > 0) {
    style.set("width", ToPx(width_));
  } else {
    style.set("width", std::string("100px"));
  }
  if (height_ > 0) {
    style.set("height", ToPx(height_));
  } else {
    style.set("height", std::string("100px"));
  }
}

// C functions that JS calls for image events
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void WebImage_handleLoad(intptr_t ptr) {
    WebImage* img = reinterpret_cast<WebImage*>(ptr);
    if (img) {
      img->HandleLoad();
    }
  }

  EMSCRIPTEN_KEEPALIVE
  void WebImage_handleError(intptr_t ptr) {
    WebImage* img = reinterpret_cast<WebImage*>(ptr);
    if (img) {
      img->HandleError();
    }
  }
}
