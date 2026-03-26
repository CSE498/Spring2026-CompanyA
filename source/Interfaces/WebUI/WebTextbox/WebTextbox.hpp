/**
 * WebTextbox.hpp
 * Declaration of the WebTextbox class for styled text elements in the WebUI.
 *
 * This class represents a DOM-backed text element used in the CSE 498 WebUI
 * subsystem. WebTextbox provides a high-level interface for manipulating and
 * rendering styled text within a browser environment compiled through
 * Emscripten/WebAssembly.
 *
 * Additional Note:
 *      Portions of formatting, documentation, and cleanup were assisted by
 *      AI tooling to improve consistency and readability.
 *
 * All project classes must reside in the cse498 namespace.
 *
 * author Lance Motowski
 * date   Spring 2026
 */

#pragma once

#include <emscripten/val.h>
#include <optional>
#include <string>

#include "../WebLayout/WebLayout.hpp"

#include "../internal/IDomElement.hpp"

namespace cse498 {

class WebTextbox : public IDomElement {
public:
  struct RectPx {
    double x{0.0};
    double y{0.0};
    double w{0.0};
    double h{0.0};
  };

  enum class TextAlign { Left, Center, Right };

  // -------- Constructors / Move semantics --------
  WebTextbox();
  explicit WebTextbox(const std::string &initial_text);

  WebTextbox(const WebTextbox &) = delete;
  WebTextbox &operator=(const WebTextbox &) = delete;

  WebTextbox(WebTextbox &&other) noexcept;
  WebTextbox &operator=(WebTextbox &&other) noexcept;

  ~WebTextbox() override;

  // -------- Text API --------
  void SetText(const std::string &text);
  void AppendText(const std::string &text);
  std::string GetText() const;
  void Clear();

  // -------- Formatting / Style API --------
  void SetFontFamily(const std::string &family);
  void SetFallbackFontFamily(const std::string &fallback_family);
  void SetFontSize(float size_px);
  void SetLineHeight(float line_height_px);
  void SetColor(const std::string &css_color);
  void SetBold(bool enabled);
  void SetItalic(bool enabled);
  void SetAlignment(TextAlign alignment); // left/center/right
  void SetMaxWidth(float width_px);
  void SetWrap(bool enabled);
  void SetBackgroundColor(const std::string &css_color);
  void ClearBackgroundColor();

  // -------- Bounding box --------
  RectPx GetBoundingBoxPx() const;
  double GetWidthPx() const;
  double GetHeightPx() const;

  // -------- Visibility --------
  void Show();
  void Hide();
  bool IsVisible() const;

  // -------- IDomElement overrides --------
  void MountToLayout(WebLayout &parent,
                     Alignment align = Alignment::None) override;

  void Unmount() override;
  void SyncFromModel() override;
  const std::string &Id() const override;

private:
  // -------- Internal helper methods --------
  void ApplyText();
  void ApplyStyles();
  void ApplyAlignment(Alignment align);

  std::string mText; // Current text content

  bool mIsVisible{true}; // Visibility state

  std::string mRequestedFontFamily; // User-requested font family (may be empty)
  std::string mFallbackFontFamily;  // Fallback font family (may be empty)

  float mFontSizePx{16.0f};  // Default font size in pixels
  float mLineHeightPx{0.0f}; // Line height in pixels (0 = normal)

  std::string mColor{"#000000"}; // Default text color (black)
  bool mBold{false};             // Bold style flag
  bool mItalic{false};           // Italic style flag
  std::string mTextAlign{"left"};

  float mMaxWidthPx{0.0f}; // 0 means no max width
  bool mWrap{true};        // Whether text should wrap when max width is set

  // Optional background color; if not set, background is transparent
  std::optional<std::string> mBackgroundColor = std::nullopt;

  // DOM element representing this textbox; null until mounted
  emscripten::val mElement{emscripten::val::null()};

  std::string mId; // Unique ID for this textboxs root DOM element
  Alignment mAlign{Alignment::None}; // Alignment within parent layout

  static int mNextIdCounter;
};

} // namespace cse498