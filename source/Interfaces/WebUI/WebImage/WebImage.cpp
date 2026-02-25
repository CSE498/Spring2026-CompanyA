#include "WebImage.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <cassert>
#include <iostream>
#include <emscripten.h>

using emscripten::val;

int WebImage::mNextIdCounter = 1;

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
    : mSrc(src),
      mAltText(alt_text),
      mElement(val::null()) {
  // Generate a unique ID for this element
  mId = "webimage-" + std::to_string(mNextIdCounter++);

  val doc = GetDocument();
  mElement = doc.call<val>("createElement", std::string("img"));
  mElement.set("id", mId);
  mElement.set("src", mSrc);
  mElement.set("alt", mAltText);
  doc["body"].call<void>("appendChild", mElement);

  AttachListeners();
}

WebImage::~WebImage() {
  if (!mElement.isNull()) {
    val parent = mElement["parentNode"];
    if (!parent.isNull() && !parent.isUndefined()) {
      parent.call<void>("removeChild", mElement);
    }
    mElement = val::null();
  }
}

WebImage::WebImage(WebImage&& other) noexcept
    : mSrc(std::move(other.mSrc)),
      mAltText(std::move(other.mAltText)),
      mWidth(other.mWidth),
      mHeight(other.mHeight),
      mOpacity(other.mOpacity),
      mIsVisible(other.mIsVisible),
      mIsLoaded(other.mIsLoaded),
      mHasError(other.mHasError),
      mErrorMode(other.mErrorMode),
      mPlaceholderColor(std::move(other.mPlaceholderColor)),
      mOnLoadCallback(std::move(other.mOnLoadCallback)),
      mOnErrorCallback(std::move(other.mOnErrorCallback)),
      mElement(other.mElement),
      mId(std::move(other.mId)) {
  other.mElement = val::null();
  other.mWidth = 0;
  other.mHeight = 0;
  other.mOpacity = 1.0;
  other.mIsVisible = false;
  other.mIsLoaded = false;
  other.mHasError = false;
}

WebImage& WebImage::operator=(WebImage&& other) noexcept {
  if (this != &other) {
    // Clean up current element
    if (!mElement.isNull()) {
      val parent = mElement["parentNode"];
      if (!parent.isNull() && !parent.isUndefined()) {
        parent.call<void>("removeChild", mElement);
      }
    }

    mSrc = std::move(other.mSrc);
    mAltText = std::move(other.mAltText);
    mWidth = other.mWidth;
    mHeight = other.mHeight;
    mOpacity = other.mOpacity;
    mIsVisible = other.mIsVisible;
    mIsLoaded = other.mIsLoaded;
    mHasError = other.mHasError;
    mErrorMode = other.mErrorMode;
    mPlaceholderColor = std::move(other.mPlaceholderColor);
    mOnLoadCallback = std::move(other.mOnLoadCallback);
    mOnErrorCallback = std::move(other.mOnErrorCallback);
    mElement = other.mElement;
    mId = std::move(other.mId);

    other.mElement = val::null();
    other.mWidth = 0;
    other.mHeight = 0;
    other.mOpacity = 1.0;
    other.mIsVisible = false;
    other.mIsLoaded = false;
    other.mHasError = false;
  }
  return *this;
}

// ----- Source & Metadata -----

void WebImage::SetSource(const std::string& src) {
  mSrc = src;
  mIsLoaded = false;
  mHasError = false;
  if (!mElement.isNull()) {
    // Reset to <img> in case we had replaced it with a placeholder <div>
    mElement.set("src", mSrc);
    mElement["style"].set("backgroundColor", std::string(""));
  }
}

std::string WebImage::GetSource() const {
  return mSrc;
}

void WebImage::SetAltText(const std::string& alt_text) {
  mAltText = alt_text;
  if (!mElement.isNull()) {
    mElement.set("alt", mAltText);
  }
}

std::string WebImage::GetAltText() const {
  return mAltText;
}

// ----- Sizing -----

void WebImage::SetSize(int width_px, int height_px) {
  assert(width_px >= 0 && "SetSize: width must be non-negative");
  assert(height_px >= 0 && "SetSize: height must be non-negative");
  mWidth = width_px;
  mHeight = height_px;
  if (!mElement.isNull()) {
    if (width_px > 0) {
      mElement["style"].set("width", ToPx(width_px));
    }
    if (height_px > 0) {
      mElement["style"].set("height", ToPx(height_px));
    }
    // Stretch to exact dimensions (no aspect ratio preservation)
    mElement["style"].set("objectFit", std::string("fill"));
  }
}

void WebImage::Resize(int width_px, int height_px, bool maintain_aspect_ratio) {
  assert(width_px > 0 && "Resize: width must be positive");
  assert(height_px > 0 && "Resize: height must be positive");
  mWidth = width_px;
  mHeight = height_px;
  if (!mElement.isNull()) {
    mElement["style"].set("width", ToPx(width_px));
    mElement["style"].set("height", ToPx(height_px));
    // "contain" scales to fit within the box keeping aspect ratio
    // "fill" stretches to fill exact dimensions
    mElement["style"].set("objectFit",
        std::string(maintain_aspect_ratio ? "contain" : "fill"));
  }
}

int WebImage::GetWidth() const {
  return mWidth;
}

int WebImage::GetHeight() const {
  return mHeight;
}

// ----- Opacity / Transparency -----

void WebImage::SetOpacity(double alpha) {
  assert(alpha >= 0.0 && alpha <= 1.0 && "SetOpacity: alpha must be in [0.0, 1.0]");
  mOpacity = alpha;
  if (!mElement.isNull()) {
    mElement["style"].set("opacity", std::to_string(alpha));
  }
}

double WebImage::GetOpacity() const {
  return mOpacity;
}

// ----- Visibility -----

void WebImage::Show() {
  mIsVisible = true;
  if (!mElement.isNull()) {
    mElement["style"].set("display", std::string(""));
  }
}

void WebImage::Hide() {
  mIsVisible = false;
  if (!mElement.isNull()) {
    mElement["style"].set("display", std::string("none"));
  }
}

bool WebImage::IsVisible() const {
  return mIsVisible;
}

// ----- Loading State & Error Handling -----

void WebImage::MarkLoaded(bool loaded) {
  mIsLoaded = loaded;
}

bool WebImage::IsLoaded() const {
  return mIsLoaded;
}

bool WebImage::HasError() const {
  return mHasError;
}

void WebImage::SetOnLoadCallback(std::function<void()> callback) {
  mOnLoadCallback = std::move(callback);
}

void WebImage::SetOnErrorCallback(std::function<void()> callback) {
  mOnErrorCallback = std::move(callback);
}

void WebImage::SetErrorMode(ImageErrorMode mode) {
  mErrorMode = mode;
}

void WebImage::SetPlaceholderColor(const std::string& css_color) {
  mPlaceholderColor = css_color;
}

// ----- IDomElement Interface -----

void WebImage::MountToLayout(WebLayout& parent, Alignment align) {
  parent.AddElement(this, align);
}

void WebImage::Unmount() {
  val parent = mElement["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", mElement);
  }
}

void WebImage::SyncFromModel() {
  // Re-apply all properties to DOM element
  if (mElement.isNull()) return;

  mElement.set("src", mSrc);
  mElement.set("alt", mAltText);

  if (mWidth > 0) {
    mElement["style"].set("width", ToPx(mWidth));
  }
  if (mHeight > 0) {
    mElement["style"].set("height", ToPx(mHeight));
  }

  mElement["style"].set("opacity", std::to_string(mOpacity));
  mElement["style"].set("display",
      std::string(mIsVisible ? "" : "none"));
}

const std::string& WebImage::Id() const {
  return mId;
}

// ----- ICanvasElement Interface -----

void WebImage::Draw(WebCanvas& canvas) {
  // Canvas drawing implementation
  // This is a stub - actual implementation would use canvas 2D context
  // to draw the image at the specified position and size.
  // For now, WebImage primarily supports DOM-based rendering.
  (void)canvas;  // Suppress unused parameter warning
}

// ----- Event Handlers -----

void WebImage::HandleLoad() {
  mIsLoaded = true;
  mHasError = false;
  if (mOnLoadCallback) {
    mOnLoadCallback();
  }
}

void WebImage::HandleError() {
  mHasError = true;
  mIsLoaded = false;
  std::cerr << "WebImage error: failed to load image '" << mSrc
            << "' (id=" << mId << ")" << std::endl;

  if (mErrorMode == ImageErrorMode::BlankRect) {
    ApplyPlaceholder();
  }
  // NoOp: do nothing, leave the broken image as-is

  if (mOnErrorCallback) {
    mOnErrorCallback();
  }
}

// ----- Private Helpers -----

void WebImage::AttachListeners() {
  if (mElement.isNull()) return;

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
  }, mElement.as_handle(), reinterpret_cast<intptr_t>(self));
}

void WebImage::ApplyPlaceholder() {
  if (mElement.isNull()) return;

  // Hide the broken image icon by removing the src
  mElement.set("src", std::string(""));

  // Show a colored rectangle instead
  val style = mElement["style"];
  style.set("backgroundColor", mPlaceholderColor);
  style.set("display", std::string("inline-block"));

  // Ensure the placeholder has visible dimensions
  if (mWidth > 0) {
    style.set("width", ToPx(mWidth));
  } else {
    style.set("width", std::string("100px"));
  }
  if (mHeight > 0) {
    style.set("height", ToPx(mHeight));
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
