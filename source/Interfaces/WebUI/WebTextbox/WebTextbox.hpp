#ifndef WEBTEXTBOX_HPP_
#define WEBTEXTBOX_HPP_

#include <functional>
#include <string>
#include <emscripten/val.h>

#include "IDomElement.hpp"   // same base used by WebImage
#include "WebLayout.hpp"     // for mountToLayout signature

/// Manages an HTML text element (<div>) from C++ via Emscripten.
/// Rendered as a DOM element via WebLayout.
class WebTextbox : public IDomElement {
 public:

  // RectPx is just a lightweight struct to hold the pixel-based bounding rectangle of a DOM element.
  struct RectPx {
    double x = 0.0;
    double y = 0.0;
    double w = 0.0;
    double h = 0.0;
  };

  /* ----- Constuctor / Destructor / Move semantics ----- */
  explicit WebTextbox(const std::string& initial_text = "");
  ~WebTextbox();

  WebTextbox(const WebTextbox&) = delete;
  WebTextbox& operator=(const WebTextbox&) = delete;

  WebTextbox(WebTextbox&& other) noexcept;
  WebTextbox& operator=(WebTextbox&& other) noexcept;

  /* ----- Text ----- */
  void SetText(const std::string& text);
  void AppendText(const std::string& text);
  std::string GetText() const;
  void Clear();

  /* ----- Formatting ----- */
  void SetFontFamily(const std::string& family);
  void SetFallbackFontFamily(const std::string& fallback_family); 
  void SetFontSize(float size_px);
  void SetLineHeight(float line_height_px);                       
  void SetColor(const std::string& css_color);
  void SetBold(bool enabled);
  void SetItalic(bool enabled);
  void SetAlignment(const std::string& alignment);
  void SetMaxWidth(float width_px);
  void SetWrap(bool enabled);                   

  /* ----- Debug and layout visualization ----- */
  void SetBackgroundColor(const std::string& css_color); 
  void ClearBackgroundColor();                           

  /* ----- Bounding box ----- */
  RectPx GetBoundingBoxPx() const; 
  double GetWidthPx() const;       
  double GetHeightPx() const;     

  /* ----- Visibility ----- */
  void Show();
  void Hide();
  bool IsVisible() const;

  /* ----- IDomElement Interface ----- */
  void MountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
  void Unmount() override;
  void SyncFromModel() override;
  const std::string& Id() const override;

 private:
  void ApplyText();
  void ApplyStyles();
  void ApplyAlignment(Alignment align);

  std::string mText;
  bool mIsVisible = true;

  // font/fallback behavior
  std::string mRequestedFontFamily;
  std::string mFallbackFontFamily = "sans-serif";

  // basic style state
  float mFontSizePx = 16.0f;
  float mLineHeightPx = 0.0f; // 0 let browser choose
  std::string mColor = "#000";
  bool mBold = false;
  bool mItalic = false;
  std::string mTextAlign = "left";
  float mMaxWidthPx = 0.0f; // 0 unset
  bool mWrap = true;
  std::string mBackgroundColor; // transparent

  // DOM and identity
  emscripten::val mElement;
  std::string mId;
  Alignment mAlign = Alignment::Start;

  static int mNextIdCounter;
};

#endif  // WEBTEXTBOX_HPP_