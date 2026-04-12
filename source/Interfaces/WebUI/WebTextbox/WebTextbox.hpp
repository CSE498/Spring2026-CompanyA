/**
 * @file WebTextbox.hpp
 * @brief Declaration of the WebTextbox class for styled text elements in the WebUI.
 *
 * This class represents a DOM-backed text element used in the CSE 498 WebUI
 * subsystem. WebTextbox provides a high-level interface for manipulating and
 * rendering styled text within a browser environment compiled through
 * Emscripten/WebAssembly.
 *
 * @note Portions of formatting, documentation, and cleanup were assisted by
 *       AI tooling to improve consistency and readability.
 *
 * All project classes must reside in the cse498 namespace.
 *
 * @author Lance Motowski
 * @date Spring 2026
 */

#pragma once

#include <string>
#include <optional>
#include <emscripten/val.h>

#include "../../../tools/Color.hpp"
#include "../WebLayout/WebLayout.hpp"

#include "../internal/IDomElement.hpp"
#include "../internal/ICanvasElement.hpp"

namespace cse498 {


/**
 * @class WebTextbox
 * @brief DOM-backed styled text element for use in WebUI layouts.
 *
 * WebTextbox wraps an HTML \<div\> element and exposes a C++ API for
 * setting text content, font properties, colors, alignment, max-width,
 * wrapping, and visibility. It implements IDomElement so it can be
 * mounted into and managed by WebLayout containers.
 */
class WebTextbox : public IDomElement, public ICanvasElement
{
public:
  /// @brief Rectangle in pixel units returned by bounding-box queries.
  struct RectPx
  {
    double x{0.0};  ///< Left edge of the bounding rectangle in pixels.
    double y{0.0};  ///< Top edge of the bounding rectangle in pixels.
    double w{0.0};  ///< Width of the bounding rectangle in pixels.
    double h{0.0};  ///< Height of the bounding rectangle in pixels.
  };

  /// @brief Horizontal text alignment options.
  enum class TextAlign { Left, Center, Right };

  // -------- Constructors / Move semantics --------

  /// @brief Default constructor; creates an empty textbox.
  WebTextbox();

  /// @brief Constructs a textbox with the given initial text content.
  /// @param initial_text Text to display on creation.
  explicit WebTextbox(const std::string& initial_text);

  WebTextbox(const WebTextbox&) = delete;
  WebTextbox& operator=(const WebTextbox&) = delete;

  /// @brief Move constructor; transfers DOM ownership from @p other.
  /// @param other Source WebTextbox to move from.
  WebTextbox(WebTextbox&& other) noexcept;

  /// @brief Move assignment; transfers DOM ownership from @p other.
  /// @param other Source WebTextbox to move from.
  /// @return Reference to this object.
  WebTextbox& operator=(WebTextbox&& other) noexcept;

  ~WebTextbox() override;

  // -------- Text API --------

  /// @brief Replaces the textbox content with the given string.
  /// @param text New text to display.
  void SetText(const std::string& text);

  /// @brief Appends a string to the existing textbox content.
  /// @param text Text to append.
  void AppendText(const std::string& text);

  /// @brief Returns the current text content of the textbox.
  /// @return Current text string.
  std::string GetText() const;

  /// @brief Clears all text content from the textbox.
  void Clear();

  // -------- Formatting / Style API --------

  /// @brief Sets the primary CSS font family.
  /// @param family Font family name (e.g., "Arial").
  void SetFontFamily(const std::string& family);

  /// @brief Sets the fallback CSS font family appended after the primary.
  /// @param fallback_family Fallback font family name.
  void SetFallbackFontFamily(const std::string& fallback_family);

  /// @brief Sets the font size in pixels; must be > 0.
  /// @param size_px Font size in pixels.
  void SetFontSize(float size_px);

  /// @brief Sets the line height in pixels; must be > 0.
  /// @param line_height_px Line height in pixels.
  void SetLineHeight(float line_height_px);

  /// @brief Sets the CSS text color.
  /// @param css_color Valid CSS color string.
  void SetColor(const std::string& css_color);

  /// @brief Enables or disables bold font weight.
  /// @param enabled True to bold, false for normal weight.
  void SetBold(bool enabled);

  /// @brief Enables or disables italic font style.
  /// @param enabled True for italic, false for normal.
  void SetItalic(bool enabled);

  /// @brief Sets horizontal text alignment (left/center/right).
  /// @param alignment Desired TextAlign value.
  void SetAlignment(TextAlign alignment);

  /// @brief Sets the maximum width of the textbox in pixels; must be > 0.
  /// @param width_px Maximum width in pixels.
  void SetMaxWidth(float width_px);

  /// @brief Controls whether text wraps at the max-width boundary.
  /// @param enabled True to enable wrapping (pre-wrap), false to disable (pre).
  void SetWrap(bool enabled);

  /// @brief Sets an explicit background color for the textbox.
  /// @param css_color Valid CSS color string.
  void SetBackgroundColor(const std::string& css_color);

  /// @brief Clears any previously set background color (reverts to transparent).
  void ClearBackgroundColor();

  // -------- Bounding box --------

  /// @brief Returns the bounding box of the DOM element in pixels.
  /// @return RectPx with x, y, w, h; zero-initialized if not mounted.
  RectPx GetBoundingBoxPx() const;

  /// @brief Returns the width of the bounding box in pixels.
  /// @return Width in pixels, or 0 if not mounted.
  double GetWidthPx() const;

  /// @brief Returns the height of the bounding box in pixels.
  /// @return Height in pixels, or 0 if not mounted.
  double GetHeightPx() const;

  // -------- Visibility --------

  /// @brief Makes the textbox visible.
  void Show();

  /// @brief Hides the textbox.
  void Hide();

  /// @brief Returns whether the textbox is currently visible.
  /// @return True if visible; false if hidden.
  bool IsVisible() const;

  // -------- IDomElement overrides --------

  /// @brief Mounts this textbox into the given parent layout.
  /// @param parent Parent WebLayout to attach to.
  /// @param align  Alignment within the parent container.
  void MountToLayout(WebLayout& parent, Alignment align = Alignment::None) override;

  /// @brief Synchronizes the DOM element with the current model state.
  void SyncFromModel() override;

  // -------- ICanvasElement overrides --------

  /// @brief Sets the canvas-space position used when drawing this textbox.
  /// @param x Canvas x coordinate (left edge of the text baseline).
  /// @param y Canvas y coordinate (baseline y).
  void SetCanvasPosition(float x, float y);

  /// @brief Draws the textbox text onto the given WebCanvas using the current
  ///        font size and color settings.
  /// @param canvas The WebCanvas to draw onto.
  void Draw(WebCanvas& canvas) override;

private:
  
  // -------- Internal helper methods --------

  /// @brief Applies the current text content to the DOM element.
  void ApplyText();

  /// @brief Applies all style properties to the DOM element.
  void ApplyStyles();

  /// @brief Applies the given alignment to the DOM element's align-self style.
  /// @param align Layout alignment hint.
  void ApplyAlignment(Alignment align);

  std::string mText;                      ///< Current text content.

  bool mIsVisible{true};                  ///< Visibility state.

  std::string mRequestedFontFamily;       ///< User-requested font family (may be empty).
  std::string mFallbackFontFamily;        ///< Fallback font family (may be empty).

  float mFontSizePx{16.0f};               ///< Default font size in pixels.
  float mLineHeightPx{0.0f};              ///< Line height in pixels (0 = normal).

  std::string mColor{Color::FromRGB255(0, 0, 0).ToHex()};  ///< Default text color (black).
  bool mBold{false};                      ///< Bold style flag.
  bool mItalic{false};                    ///< Italic style flag.
  std::string mTextAlign{"left"};         ///< CSS text-align value.

  float mMaxWidthPx{0.0f};                ///< 0 means no max width.
  bool mWrap{true};                       ///< Whether text wraps when max width is set.

  
  // Optional background color; if not set, background is transparent
  std::optional<std::string> mBackgroundColor = std::nullopt;

  Alignment mAlign{Alignment::None};      ///< Alignment within parent layout.

  float mCanvasX = 0.0f;  ///< Canvas draw position x (pixels).
  float mCanvasY = 0.0f;  ///< Canvas draw position y (pixels).

  static int mNextIdCounter;  ///< Counter for generating unique DOM ids.
};

} // namespace cse498
