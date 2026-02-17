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
  void mountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
  void unmount() override;
  void syncFromModel() override;
  const std::string& Id() const override;

 private:
  void ApplyText();
  void ApplyStyles();
  void ApplyAlignment(Alignment align);

  std::string text_;
  bool is_visible_ = true;

  // font/fallback behavior
  std::string requested_font_family_;
  std::string fallback_font_family_ = "sans-serif";

  // basic style state
  float font_size_px_ = 16.0f;
  float line_height_px_ = 0.0f; // 0 let browser choose
  std::string color_ = "#000";
  bool bold_ = false;
  bool italic_ = false;
  std::string text_align_ = "left";
  float max_width_px_ = 0.0f; // 0 unset
  bool wrap_ = true;
  std::string background_color_; // transparent

  // DOM and identity
  emscripten::val element_;
  std::string id_;
  Alignment align_ = Alignment::Start;

  static int next_id_counter_;
};

#endif  // WEBTEXTBOX_HPP_