#include "WebImage.hpp"
#include "../WebLayout/WebLayout.hpp"
#include <cassert>
#include <iostream>
#include <unordered_map>
#include <emscripten.h>

using emscripten::val;

namespace {

// Registry maps integer IDs to WebImage pointers, avoiding unsafe pointer-to-int casts
// when forwarding events from JavaScript back to C++.
std::unordered_map<int, cse498::WebImage*> sImageRegistry;
int sNextRegistryId = 0;

int RegisterImage(cse498::WebImage* img) {
  int id = sNextRegistryId++;
  sImageRegistry[id] = img;
  return id;
}

void UnregisterImage(int id) {
  sImageRegistry.erase(id);
}

void UpdateRegistryEntry(int id, cse498::WebImage* img) {
  sImageRegistry[id] = img;
}

cse498::WebImage* LookupImage(int id) {
  auto it = sImageRegistry.find(id);
  return it != sImageRegistry.end() ? it->second : nullptr;
}

}  // anonymous namespace

namespace cse498 {

static constexpr int kDefaultPlaceholderPx = 100;

int WebImage::mNextIdCounter = 1;

// ----- Private static helpers -----

/// Returns the browser document object via Emscripten.
val WebImage::GetDocument() {
  return val::global("document");
}

/// Converts an integer to a CSS pixel string, e.g. 42 -> "42px".
std::string WebImage::ToPx(int value) {
  return std::to_string(value) + "px";
}

// ----- Private helpers -----

/// Removes the element from its parent DOM node if currently attached.
void WebImage::RemoveFromDom() {
  if (mElement.isNull()) return;
  val parent = mElement["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", mElement);
  }
}

/// Removes element from DOM, deregisters from the event registry, and nulls the element.
void WebImage::CleanupElement() {
  RemoveFromDom();
  if (mRegistryId >= 0) {
    UnregisterImage(mRegistryId);
    mRegistryId = -1;
  }
  mElement = val::null();
}

// ----- Construction / Destruction -----

/// Constructs a WebImage, creates the <img> DOM element, and attaches event listeners.
WebImage::WebImage(const std::string& src, const std::string& alt_text)
    : mSrc(src),
      mAltText(alt_text),
      mElement(val::null()) {
  mId = "webimage-" + std::to_string(mNextIdCounter++);
  mRegistryId = RegisterImage(this);

  val doc = GetDocument();
  mElement = doc.call<val>("createElement", std::string("img"));
  mElement.set("id", mId);
  mElement.set("src", mSrc);
  mElement.set("alt", mAltText);
  doc["body"].call<void>("appendChild", mElement);

  AttachListeners();
}

/// Cleans up the DOM element and deregisters from the event registry.
WebImage::~WebImage() {
  CleanupElement();
}

/// Move-constructs a WebImage, transferring DOM ownership from other.
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
      mId(std::move(other.mId)),
      mRegistryId(other.mRegistryId) {
  other.mElement = val::null();
  other.mRegistryId = -1;
  other.mWidth = 0;
  other.mHeight = 0;
  other.mOpacity = 1.0;
  other.mIsVisible = false;
  other.mIsLoaded = false;
  other.mHasError = false;

  if (mRegistryId >= 0) {
    UpdateRegistryEntry(mRegistryId, this);
  }
}

/// Move-assigns a WebImage, cleaning up the current element first.
WebImage& WebImage::operator=(WebImage&& other) noexcept {
  if (this != &other) {
    CleanupElement();

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
    mRegistryId = other.mRegistryId;

    other.mElement = val::null();
    other.mRegistryId = -1;
    other.mWidth = 0;
    other.mHeight = 0;
    other.mOpacity = 1.0;
    other.mIsVisible = false;
    other.mIsLoaded = false;
    other.mHasError = false;

    if (mRegistryId >= 0) {
      UpdateRegistryEntry(mRegistryId, this);
    }
  }
  return *this;
}

// ----- Source & Metadata -----

/// Updates the image source URL and resets loading state.
void WebImage::SetSource(const std::string& src) {
  mSrc = src;
  mIsLoaded = false;
  mHasError = false;
  if (!mElement.isNull()) {
    mElement.set("src", mSrc);
    mElement["style"].set("backgroundColor", std::string(""));
  }
}

/// Returns the current image source URL.
std::string WebImage::GetSource() const {
  return mSrc;
}

/// Updates the alt text on the DOM element.
void WebImage::SetAltText(const std::string& alt_text) {
  mAltText = alt_text;
  if (!mElement.isNull()) {
    mElement.set("alt", mAltText);
  }
}

/// Returns the current alt text.
std::string WebImage::GetAltText() const {
  return mAltText;
}

// ----- Sizing -----

/// Sets the display size in pixels and applies object-fit: fill.
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
    mElement["style"].set("objectFit", std::string("fill"));
  }
}

/// Resizes the image, optionally preserving aspect ratio via object-fit.
void WebImage::Resize(int width_px, int height_px, bool maintain_aspect_ratio) {
  assert(width_px > 0 && "Resize: width must be positive");
  assert(height_px > 0 && "Resize: height must be positive");
  mWidth = width_px;
  mHeight = height_px;
  if (!mElement.isNull()) {
    mElement["style"].set("width", ToPx(width_px));
    mElement["style"].set("height", ToPx(height_px));
    mElement["style"].set("objectFit",
        std::string(maintain_aspect_ratio ? "contain" : "fill"));
  }
}

/// Returns the current display width.
int WebImage::GetWidth() const {
  return mWidth;
}

/// Returns the current display height.
int WebImage::GetHeight() const {
  return mHeight;
}

// ----- Opacity / Transparency -----

/// Applies the given opacity (clamped to [0.0, 1.0]) to the DOM element.
void WebImage::SetOpacity(double alpha) {
  assert(alpha >= 0.0 && alpha <= 1.0 && "SetOpacity: alpha must be in [0.0, 1.0]");
  mOpacity = alpha;
  if (!mElement.isNull()) {
    mElement["style"].set("opacity", std::to_string(alpha));
  }
}

/// Returns the current opacity value.
double WebImage::GetOpacity() const {
  return mOpacity;
}

// ----- Visibility -----

/// Makes the image visible by clearing the CSS display property.
void WebImage::Show() {
  mIsVisible = true;
  if (!mElement.isNull()) {
    mElement["style"].set("display", std::string(""));
  }
}

/// Hides the image by setting display: none.
void WebImage::Hide() {
  mIsVisible = false;
  if (!mElement.isNull()) {
    mElement["style"].set("display", std::string("none"));
  }
}

/// Returns whether the image is currently visible.
bool WebImage::IsVisible() const {
  return mIsVisible;
}

// ----- Loading State & Error Handling -----

/// Manually sets the loaded flag (used for tracking async load state).
void WebImage::MarkLoaded(bool loaded) {
  mIsLoaded = loaded;
}

/// Returns true if the image has finished loading.
bool WebImage::IsLoaded() const {
  return mIsLoaded;
}

/// Returns true if the image source failed to load.
bool WebImage::HasError() const {
  return mHasError;
}

/// Registers a callback invoked when the image loads successfully.
void WebImage::SetOnLoadCallback(std::function<void()> callback) {
  mOnLoadCallback = std::move(callback);
}

/// Registers a callback invoked when the image fails to load.
void WebImage::SetOnErrorCallback(std::function<void()> callback) {
  mOnErrorCallback = std::move(callback);
}

/// Configures the error-handling strategy (BlankRect or NoOp).
void WebImage::SetErrorMode(ImageErrorMode mode) {
  mErrorMode = mode;
}

/// Sets the CSS color used for the BlankRect placeholder.
void WebImage::SetPlaceholderColor(const std::string& css_color) {
  mPlaceholderColor = css_color;
}

// ----- IDomElement Interface -----

/// Mounts this image into a parent WebLayout container with the given alignment.
void WebImage::MountToLayout(WebLayout& parent, Alignment align) {
  parent.AddElement(this, align);
}

/// Removes this image from its parent DOM node.
void WebImage::Unmount() {
  RemoveFromDom();
}

/// Re-applies all tracked properties to the underlying DOM element.
void WebImage::SyncFromModel() {
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

/// Returns this element's unique DOM id.
const std::string& WebImage::Id() const {
  return mId;
}

// ----- ICanvasElement Interface -----

/// Draws the image on a WebCanvas (stub; primary rendering is DOM-based).
void WebImage::Draw(WebCanvas& canvas) {
  (void)canvas;
}

// ----- Event Handlers -----

/// Called when the image finishes loading; sets state and fires the load callback.
void WebImage::HandleLoad() {
  mIsLoaded = true;
  mHasError = false;
  if (mOnLoadCallback) {
    mOnLoadCallback();
  }
}

/// Called when the image fails to load; applies placeholder if configured and fires the error callback.
void WebImage::HandleError() {
  mHasError = true;
  mIsLoaded = false;
  std::cerr << "WebImage error: failed to load image '" << mSrc
            << "' (id=" << mId << ")" << std::endl;

  if (mErrorMode == ImageErrorMode::BlankRect) {
    ApplyPlaceholder();
  }

  if (mOnErrorCallback) {
    mOnErrorCallback();
  }
}

// ----- Private Helpers -----

/// Attaches JS load/error event listeners that forward to HandleLoad/HandleError via registry ID.
void WebImage::AttachListeners() {
  if (mElement.isNull()) return;

  int regId = mRegistryId;

  EM_ASM({
    var el = Emval.toValue($0);
    var registryId = $1;
    el.addEventListener("load", function() {
      Module._WebImage_handleLoad(registryId);
    });
    el.addEventListener("error", function() {
      Module._WebImage_handleError(registryId);
    });
  }, mElement.as_handle(), regId);
}

/// Replaces the broken image with a colored rectangle of the configured placeholder color.
void WebImage::ApplyPlaceholder() {
  if (mElement.isNull()) return;

  mElement.set("src", std::string(""));

  val style = mElement["style"];
  style.set("backgroundColor", mPlaceholderColor);
  style.set("display", std::string("inline-block"));

  if (mWidth > 0) {
    style.set("width", ToPx(mWidth));
  } else {
    style.set("width", ToPx(kDefaultPlaceholderPx));
  }
  if (mHeight > 0) {
    style.set("height", ToPx(mHeight));
  } else {
    style.set("height", ToPx(kDefaultPlaceholderPx));
  }
}

}  // namespace cse498

// C functions invoked from JS event listeners via the integer registry ID.
extern "C" {
  EMSCRIPTEN_KEEPALIVE
  void WebImage_handleLoad(int registry_id) {
    auto* img = LookupImage(registry_id);
    if (img) {
      img->HandleLoad();
    }
  }

  EMSCRIPTEN_KEEPALIVE
  void WebImage_handleError(int registry_id) {
    auto* img = LookupImage(registry_id);
    if (img) {
      img->HandleError();
    }
  }
}
