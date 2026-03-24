/**
 * @file WebImage.cpp
 * @brief Implementation of WebImage, a DOM-backed HTML \<img\> element for WebUI.
 *
 * Manages DOM creation, event listener attachment, loading/error state
 * tracking, placeholder rendering, and lifecycle management (mount/unmount/sync)
 * for HTML \<img\> elements. An integer registry is used to safely forward
 * JavaScript load and error events back to C++ without unsafe pointer casts.
 *
 */

#include "WebImage.hpp"
#include "../WebCanvas/WebCanvas.hpp"
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

/// @brief Registers a WebImage instance and returns its unique integer id.
/// @param img Pointer to the WebImage to register.
/// @return Integer id that can be passed to JS event listeners.
int RegisterImage(cse498::WebImage* img) {
  int id = sNextRegistryId++;
  sImageRegistry[id] = img;
  return id;
}

/// @brief Removes the registry entry for the given id.
/// @param id Registry id previously returned by RegisterImage().
void UnregisterImage(int id) {
  sImageRegistry.erase(id);
}

/// @brief Updates the registry entry for @p id to point to @p img (used after move).
/// @param id  Registry id of the entry to update.
/// @param img New WebImage pointer to store.
void UpdateRegistryEntry(int id, cse498::WebImage* img) {
  sImageRegistry[id] = img;
}

/// @brief Looks up the WebImage registered under @p id.
/// @param id Registry id to look up.
/// @return Pointer to the registered WebImage, or nullptr if not found.
cse498::WebImage* LookupImage(int id) {
  auto it = sImageRegistry.find(id);
  return it != sImageRegistry.end() ? it->second : nullptr;
}

/// @brief Returns true when the DOM element is usable; otherwise logs a warning.
/// @param element DOM handle to validate.
/// @param id      Logical WebImage id used in the warning.
/// @param action  Description of the skipped action.
/// @return True if the DOM element is neither null nor undefined.
bool EnsureElementAvailable(const val& element,
                            const std::string& id,
                            const char* action) {
  if (!element.isNull() && !element.isUndefined()) {
    return true;
  }

  const std::string resolved_id = id.empty() ? "<uninitialized>" : id;
  cse498::GetConsole().call<void>(
      "warn",
      std::string("WebImage '") + resolved_id
          + "' skipped " + action + " because its DOM element is unavailable.");
  return false;
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

/// Removes element from DOM, deregisters from the event registry, and nulls the element.
void WebImage::CleanupElement() {
  Unmount();
  if (mRegistryId >= 0) {
    UnregisterImage(mRegistryId);
    mRegistryId = -1;
  }
  mElement = val::null();
}

// ----- Construction / Destruction -----

/// Constructs a detached WebImage, creates the <img> DOM element, and attaches
/// event listeners without mounting the element into the document body.
WebImage::WebImage(const std::string& src, const std::string& alt_text)
    : mSrc(src),
      mAltText(alt_text) {
  mId = "webimage-" + std::to_string(mNextIdCounter++);
  mRegistryId = RegisterImage(this);

  val doc = GetDocument();
  mElement = doc.call<val>("createElement", std::string("img"));
  mElement.set("id", mId);
  mElement.set("src", mSrc);
  mElement.set("alt", mAltText);

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
      mRegistryId(other.mRegistryId) {
  mId = std::move(other.mId);
  mElement = std::move(other.mElement);
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

/// Re-applies all tracked properties to the underlying DOM element.
void WebImage::SyncFromModel() {
  if (!EnsureElementAvailable(mElement, mId, "SyncFromModel")) return;

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

// ----- ICanvasElement Interface -----

/// Stores the canvas-space rectangle used by Draw().
void WebImage::SetCanvasRect(float x, float y, float w, float h) {
  mCanvasX = x;
  mCanvasY = y;
  mCanvasW = w;
  mCanvasH = h;
}

float WebImage::CanvasX() const { return mCanvasX; }
float WebImage::CanvasY() const { return mCanvasY; }
float WebImage::CanvasW() const { return mCanvasW; }
float WebImage::CanvasH() const { return mCanvasH; }

/// Draws the image onto @p canvas at the position set by SetCanvasRect().
/// If the image failed to load and the error mode is BlankRect, a colored
/// rectangle is drawn as a placeholder instead.
void WebImage::Draw(WebCanvas& canvas) {
  if (mHasError && mErrorMode == ImageErrorMode::BlankRect) {
    float w = (mCanvasW > 0) ? mCanvasW : static_cast<float>(mWidth > 0 ? mWidth : 100);
    float h = (mCanvasH > 0) ? mCanvasH : static_cast<float>(mHeight > 0 ? mHeight : 100);
    canvas.DrawRect(mCanvasX, mCanvasY, w, h, mPlaceholderColor);
    return;
  }
  canvas.DrawImage(mSrc, mCanvasX, mCanvasY, mCanvasW, mCanvasH);
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
  if (!EnsureElementAvailable(mElement, mId, "AttachListeners")) return;

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
  if (!EnsureElementAvailable(mElement, mId, "ApplyPlaceholder")) return;

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
  /// @brief C trampoline called by the JS load event listener.
  /// @param registry_id Integer id of the WebImage to notify.
  EMSCRIPTEN_KEEPALIVE
  void WebImage_handleLoad(int registry_id) {
    auto* img = LookupImage(registry_id);
    if (img) {
      img->HandleLoad();
    }
  }

  /// @brief C trampoline called by the JS error event listener.
  /// @param registry_id Integer id of the WebImage to notify.
  EMSCRIPTEN_KEEPALIVE
  void WebImage_handleError(int registry_id) {
    auto* img = LookupImage(registry_id);
    if (img) {
      img->HandleError();
    }
  }

  /// @brief Testing hook that reports whether a registry id is still registered.
  /// @param registry_id Integer id to look up in the image registry.
  /// @return 1 if the registry id maps to a live image; otherwise 0.
  EMSCRIPTEN_KEEPALIVE
  int WebImage_registryContains(int registry_id) {
    return LookupImage(registry_id) != nullptr ? 1 : 0;
  }
}
