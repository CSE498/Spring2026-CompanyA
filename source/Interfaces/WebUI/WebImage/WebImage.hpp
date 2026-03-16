/**
 * @file WebImage.hpp
 * @brief Declaration of WebImage, a DOM-backed HTML \<img\> element for WebUI.
 *
 * WebImage manages an HTML \<img\> element from C++ via Emscripten. It
 * implements IDomElement for DOM lifecycle management (mount/unmount/sync)
 * and ICanvasElement for optional canvas-based rendering. Positioning is
 * NOT handled by WebImage; use WebLayout (Flex/Grid/Free) to control where
 * the image appears on the page.
 *
 */

#ifndef WEBIMAGE_HPP_
#define WEBIMAGE_HPP_

#include <string>
#include <functional>
#include <emscripten/val.h>

#include "../internal/IDomElement.hpp"
#include "../internal/ICanvasElement.hpp"

namespace cse498 {

/// @brief Behavior when an image source fails to load.
enum class ImageErrorMode {
  BlankRect,  ///< Show a blank colored rectangle as placeholder.
  NoOp        ///< Do nothing; the element remains invisible/broken.
};

/**
 * @class WebImage
 * @brief DOM-backed HTML \<img\> element for use in WebUI layouts.
 *
 * Manages an HTML \<img\> element from C++ via Emscripten.
 * Can be rendered as a DOM element via WebLayout or drawn on a WebCanvas.
 * Positioning is NOT handled by WebImage; use WebLayout (Flex/Grid/Free)
 * to control where the image appears on the page.
 */
class WebImage : public IDomElement, public ICanvasElement {
 public:
  /// Construct a WebImage with a source URL/path and optional alt text.
  explicit WebImage(const std::string& src,
                    const std::string& alt_text = "");
  ~WebImage();

  WebImage(const WebImage&) = delete;
  WebImage& operator=(const WebImage&) = delete;

  WebImage(WebImage&& other) noexcept;
  WebImage& operator=(WebImage&& other) noexcept;

  // ----- Source & Metadata -----

  /// Set the image source (URL or asset path).
  void SetSource(const std::string& src);
  /// Get the current image source.
  [[nodiscard]] std::string GetSource() const;

  /// Set the alternative text for accessibility.
  void SetAltText(const std::string& alt_text);
  /// Get the alternative text.
  [[nodiscard]] std::string GetAltText() const;

  // ----- Sizing -----

  /// Set the display size of the image (does not preserve aspect ratio).
  /// @param width_px  Width in pixels (must be >= 0)
  /// @param height_px Height in pixels (must be >= 0)
  void SetSize(int width_px, int height_px);

  /// Resize the image. If maintain_aspect_ratio is true, the image is
  /// scaled to fit within the given bounding box while preserving its
  /// original aspect ratio (CSS object-fit: contain). Otherwise it
  /// stretches to fill the exact dimensions (CSS object-fit: fill).
  /// @param width_px              Target width in pixels (must be > 0)
  /// @param height_px             Target height in pixels (must be > 0)
  /// @param maintain_aspect_ratio If true, scale to fit; if false, stretch.
  void Resize(int width_px, int height_px, bool maintain_aspect_ratio = false);

  /// Get the display width.
  [[nodiscard]] int GetWidth() const;
  /// Get the display height.
  [[nodiscard]] int GetHeight() const;

  // ----- Opacity / Transparency -----

  /// Set the opacity of the image (0.0 = fully transparent, 1.0 = fully opaque).
  void SetOpacity(double alpha);
  /// Get the current opacity.
  [[nodiscard]] double GetOpacity() const;

  // ----- Visibility -----

  /// Show the image.
  void Show();
  /// Hide the image.
  void Hide();
  /// Check if the image is visible.
  [[nodiscard]] bool IsVisible() const;

  // ----- Loading State & Error Handling -----

  /// Check if the image has been loaded.
  [[nodiscard]] bool IsLoaded() const;
  /// Check if the image source failed to load.
  [[nodiscard]] bool HasError() const;

  /// Set a callback to be invoked when the image finishes loading.
  void SetOnLoadCallback(std::function<void()> callback);

  /// Set a callback to be invoked when the image fails to load.
  void SetOnErrorCallback(std::function<void()> callback);

  /// Set the behavior when an image fails to load.
  /// BlankRect: renders a colored placeholder rectangle.
  /// NoOp: does nothing (element stays as-is).
  void SetErrorMode(ImageErrorMode mode);

  /// Set the placeholder color shown when image fails to load
  /// (only used when error mode is BlankRect). Any valid CSS color.
  void SetPlaceholderColor(const std::string& css_color);

  // ----- IDomElement Interface -----

  void MountToLayout(WebLayout& parent, Alignment align) override;
  void Unmount() override;
  void SyncFromModel() override;
  [[nodiscard]] const std::string& Id() const override;

  // ----- ICanvasElement Interface -----

  void Draw(WebCanvas& canvas) override;

  /// Handle load event (called when image finishes loading).
  void HandleLoad();
  /// Handle error event (called when image fails to load).
  void HandleError();

 private:
  std::string mSrc;                         ///< Image source URL or asset path.
  std::string mAltText;                     ///< Alternative text for accessibility.
  int mWidth = 0;                           ///< Display width in pixels (0 = browser default).
  int mHeight = 0;                          ///< Display height in pixels (0 = browser default).
  double mOpacity = 1.0;                    ///< Opacity in [0.0, 1.0].
  bool mIsVisible = true;                   ///< Whether the image element is visible.
  bool mIsLoaded = false;                   ///< True after the image finishes loading.
  bool mHasError = false;                   ///< True if the image source failed to load.
  ImageErrorMode mErrorMode = ImageErrorMode::BlankRect;  ///< Error handling strategy.
  std::string mPlaceholderColor = "#CCCCCC";  ///< Color used for the BlankRect placeholder.
  std::function<void()> mOnLoadCallback;    ///< Callback invoked on successful load.
  std::function<void()> mOnErrorCallback;   ///< Callback invoked on load failure.
  emscripten::val mElement;                 ///< Underlying HTML \<img\> DOM element.
  std::string mId;                          ///< Unique DOM id for this image element.
  int mRegistryId = -1;                     ///< Registry id for JS event forwarding.

  static int mNextIdCounter;                ///< Counter for generating unique DOM ids.

  /// Mark the image as loaded or not (for tracking async loading).
  void MarkLoaded(bool loaded);
  /// Remove the element from its parent DOM node.
  void RemoveFromDom();
  /// Remove from DOM, deregister from event registry, and null the element.
  void CleanupElement();
  /// Attach the onload/onerror event listeners to the DOM element.
  void AttachListeners();
  /// Apply the blank-rect placeholder fallback.
  void ApplyPlaceholder();
  /// Get the browser's document object.
  static emscripten::val GetDocument();
  /// Convert an integer to a CSS pixel string (e.g. "100px").
  static std::string ToPx(int value);
};

}  // namespace cse498

#endif  // WEBIMAGE_HPP_
