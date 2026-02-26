#ifndef WEBIMAGE_HPP_
#define WEBIMAGE_HPP_

#include <string>
#include <functional>
#include <emscripten/val.h>

#include "../internal/IDomElement.hpp"
#include "../internal/ICanvasElement.hpp"

/// Behavior when an image source fails to load.
enum class ImageErrorMode {
  BlankRect,  ///< Show a blank colored rectangle as placeholder
  NoOp        ///< Do nothing; the element remains invisible/broken
};

/// Manages an HTML <img> element from C++ via Emscripten.
/// Can be rendered as a DOM element via WebLayout or drawn on a WebCanvas.
///
/// Positioning is NOT handled by WebImage. Use WebLayout (Flex/Grid/Free)
/// to control where the image appears on the page.
class WebImage : public IDomElement, public ICanvasElement {
 public:
  /// Construct a WebImage with a source URL/path and optional alt text.
  explicit WebImage(const std::string& src,
                    const std::string& alt_text = "");
  ~WebImage();

  // Each instance owns a unique DOM element
  WebImage(const WebImage&) = delete;
  WebImage& operator=(const WebImage&) = delete;

  // Transfers DOM ownership
  WebImage(WebImage&& other) noexcept;
  WebImage& operator=(WebImage&& other) noexcept;

  // ----- Source & Metadata -----

  /// Set the image source (URL or asset path).
  void SetSource(const std::string& src);
  /// Get the current image source.
  std::string GetSource() const;

  /// Set the alternative text for accessibility.
  void SetAltText(const std::string& alt_text);
  /// Get the alternative text.
  std::string GetAltText() const;

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
  int GetWidth() const;
  /// Get the display height.
  int GetHeight() const;

  // ----- Opacity / Transparency -----

  /// Set the opacity of the image (0.0 = fully transparent, 1.0 = fully opaque).
  void SetOpacity(double alpha);
  /// Get the current opacity.
  double GetOpacity() const;

  // ----- Visibility -----

  /// Show the image.
  void Show();
  /// Hide the image.
  void Hide();
  /// Check if the image is visible.
  bool IsVisible() const;

  // ----- Loading State & Error Handling -----

  /// Mark the image as loaded or not (for tracking async loading).
  void MarkLoaded(bool loaded);
  /// Check if the image has been loaded.
  bool IsLoaded() const;
  /// Check if the image source failed to load.
  bool HasError() const;

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

  void MountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
  void Unmount() override;
  void SyncFromModel() override;
  const std::string& Id() const override;

  // ----- ICanvasElement Interface -----

  void Draw(WebCanvas& canvas) override;

  /// Handle load event (called when image finishes loading).
  void HandleLoad();
  /// Handle error event (called when image fails to load).
  void HandleError();

 private:
  std::string mSrc;
  std::string mAltText;
  int mWidth = 0;
  int mHeight = 0;
  double mOpacity = 1.0;
  bool mIsVisible = true;
  bool mIsLoaded = false;
  bool mHasError = false;
  ImageErrorMode mErrorMode = ImageErrorMode::BlankRect;
  std::string mPlaceholderColor = "#CCCCCC";
  std::function<void()> mOnLoadCallback;
  std::function<void()> mOnErrorCallback;
  emscripten::val mElement;
  std::string mId;

  static int mNextIdCounter;

  /// Attach the onload/onerror event listeners to the DOM element.
  void AttachListeners();
  /// Apply the blank-rect placeholder fallback.
  void ApplyPlaceholder();
};

#endif  // WEBIMAGE_HPP_
