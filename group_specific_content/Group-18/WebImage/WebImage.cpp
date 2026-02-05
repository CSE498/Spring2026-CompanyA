#include "WebImage.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <cassert>
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
  element_["style"].set("position", std::string("absolute"));
  element_["style"].set("objectFit", std::string("contain"));
  doc["body"].call<void>("appendChild", element_);

  AttachLoadListener();
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
      x_(other.x_),
      y_(other.y_),
      maintain_aspect_ratio_(other.maintain_aspect_ratio_),
      is_visible_(other.is_visible_),
      is_loaded_(other.is_loaded_),
      on_load_callback_(std::move(other.on_load_callback_)),
      element_(other.element_),
      id_(std::move(other.id_)) {
  other.element_ = val::null();
  other.width_ = 0;
  other.height_ = 0;
  other.x_ = 0;
  other.y_ = 0;
  other.is_visible_ = false;
  other.is_loaded_ = false;
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
    x_ = other.x_;
    y_ = other.y_;
    maintain_aspect_ratio_ = other.maintain_aspect_ratio_;
    is_visible_ = other.is_visible_;
    is_loaded_ = other.is_loaded_;
    on_load_callback_ = std::move(other.on_load_callback_);
    element_ = other.element_;
    id_ = std::move(other.id_);

    other.element_ = val::null();
    other.width_ = 0;
    other.height_ = 0;
    other.x_ = 0;
    other.y_ = 0;
    other.is_visible_ = false;
    other.is_loaded_ = false;
  }
  return *this;
}

// ----- Source & Metadata -----

void WebImage::SetSource(const std::string& src) {
  src_ = src;
  is_loaded_ = false;  // Reset loaded state when source changes
  if (!element_.isNull()) {
    element_.set("src", src_);
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

// ----- Geometry -----

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
  }
}

int WebImage::GetWidth() const {
  return width_;
}

int WebImage::GetHeight() const {
  return height_;
}

void WebImage::SetPosition(int x, int y) {
  x_ = x;
  y_ = y;
  if (!element_.isNull()) {
    element_["style"].set("left", ToPx(x));
    element_["style"].set("top", ToPx(y));
  }
}

int WebImage::GetX() const {
  return x_;
}

int WebImage::GetY() const {
  return y_;
}

// ----- Aspect Ratio & Fitting -----

void WebImage::SetMaintainAspectRatio(bool enabled) {
  maintain_aspect_ratio_ = enabled;
  if (!element_.isNull()) {
    // "contain" preserves aspect ratio, "fill" stretches to fit
    element_["style"].set("objectFit",
        std::string(enabled ? "contain" : "fill"));
  }
}

bool WebImage::GetMaintainAspectRatio() const {
  return maintain_aspect_ratio_;
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

// ----- Loading State -----

void WebImage::MarkLoaded(bool loaded) {
  is_loaded_ = loaded;
}

bool WebImage::IsLoaded() const {
  return is_loaded_;
}

void WebImage::SetOnLoadCallback(std::function<void()> callback) {
  on_load_callback_ = std::move(callback);
}

// ----- IDomElement Interface -----

void WebImage::mountToLayout(WebLayout& parent, Alignment align) {
  parent.AddElement(id_, align);
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

  element_["style"].set("left", ToPx(x_));
  element_["style"].set("top", ToPx(y_));
  element_["style"].set("objectFit",
      std::string(maintain_aspect_ratio_ ? "contain" : "fill"));
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

// ----- Private Helpers -----

void WebImage::AttachLoadListener() {
  if (element_.isNull()) return;

  WebImage* self = this;

  EM_ASM({
    var el = Emval.toValue($0);
    var ptrVal = $1;
    el.addEventListener("load", function() {
      Module._WebImage_handleLoad(ptrVal);
    });
  }, element_.as_handle(), reinterpret_cast<intptr_t>(self));
}

void WebImage::HandleLoad() {
  is_loaded_ = true;
  if (on_load_callback_) {
    on_load_callback_();
  }
}

// C function that JS calls when image loads
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void WebImage_handleLoad(intptr_t ptr) {
    WebImage* img = reinterpret_cast<WebImage*>(ptr);
    if (img) {
      img->HandleLoad();
    }
  }
}
