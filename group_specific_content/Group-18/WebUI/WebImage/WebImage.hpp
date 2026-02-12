#ifndef WEBIMAGE_HPP_
#define WEBIMAGE_HPP_

#include <string>
#include <functional>
#include <emscripten/val.h>

#include "../WebLayout/IDomElement.hpp"
#include "../WebCanvas/ICanvasElement.hpp"

/// Manages an HTML <img> element from C++ via Emscripten.
/// Can be rendered as a DOM element via WebLayout or drawn on a WebCanvas.
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

  // ----- Geometry (logical size & position, in pixels) -----

  /// Set the display size of the image.
  void SetSize(int width_px, int height_px);
  /// Get the display width.
  int GetWidth() const;
  /// Get the display height.
  int GetHeight() const;

  /// Set the position of the image.
  void SetPosition(int x, int y);
  /// Get the X coordinate.
  int GetX() const;
  /// Get the Y coordinate.
  int GetY() const;

  // ----- Aspect Ratio & Fitting -----

  /// Enable or disable maintaining aspect ratio when resizing.
  void SetMaintainAspectRatio(bool enabled);
  /// Check if aspect ratio is being maintained.
  bool GetMaintainAspectRatio() const;

  // ----- Visibility -----

  /// Show the image.
  void Show();
  /// Hide the image.
  void Hide();
  /// Check if the image is visible.
  bool IsVisible() const;

  // ----- Loading State -----

  /// Mark the image as loaded or not (for tracking async loading).
  void MarkLoaded(bool loaded);
  /// Check if the image has been loaded.
  bool IsLoaded() const;

  /// Set a callback to be invoked when the image finishes loading.
  void SetOnLoadCallback(std::function<void()> callback);

  // ----- IDomElement Interface -----

  void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
  void unmount() override;
  void syncFromModel() override;
  const std::string& Id() const override;

  // ----- ICanvasElement Interface -----

  void draw(WebCanvas& canvas) override;

  /// Handle load event (called when image finishes loading).
  void HandleLoad();

 private:
  std::string src_;
  std::string alt_text_;
  int width_ = 0;
  int height_ = 0;
  int x_ = 0;
  int y_ = 0;
  bool maintain_aspect_ratio_ = true;
  bool is_visible_ = true;
  bool is_loaded_ = false;
  std::function<void()> on_load_callback_;
  emscripten::val element_;
  std::string id_;

  static int next_id_counter_;

  /// Attach the onload event listener to the DOM element.
  void AttachLoadListener();
};

#endif  // WEBIMAGE_HPP_
