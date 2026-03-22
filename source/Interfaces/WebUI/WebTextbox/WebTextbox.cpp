/**
 * @file WebTextbox.cpp
 * @brief Implementation of the WebTextbox class for styled text elements in the WebUI.
 *
 * This file contains the DOM interaction logic, style application, and
 * model-view synchronization used by the WebTextbox component within the
 * CSE 498 WebUI subsystem. Emscripten’s embind APIs are used to interface
 * with underlying JavaScript DOM objects.
 *
 * @note Portions of formatting, documentation, and cleanup were assisted by
 *       AI tooling to improve consistency and readability.
 *
 * All project classes must reside in the cse498 namespace.
 *
 * @author Lance Motowski
 * @date Spring 2026
 */

#include "WebTextbox.hpp"
#include "../WebCanvas/WebCanvas.hpp"
#include "../WebUtils.hpp"

#include <cassert>
#include <utility>
#include <emscripten/val.h>

using emscripten::val;

namespace cse498 {

int WebTextbox::mNextIdCounter = 1;


/// @brief Returns the browser window object.
static val GetWindow()   
{ 
  return val::global("window"); 
}

// ===== Constructor / Destructor / Move semantics =====

/// @brief Default constructor; delegates to WebTextbox(std::string("")).
WebTextbox::WebTextbox()
    : WebTextbox(std::string(""))
{
}

/// @brief Constructs a WebTextbox with the given initial text content.
/// @param initial_text Text to display on creation.
WebTextbox::WebTextbox(const std::string& initial_text)
    : mText(initial_text)
{
  // unique id like WebImage
  mId = "webtextbox-" + std::to_string(mNextIdCounter++);

  val doc_ = GetDocument();
  mElement = doc_.call<val>("createElement", std::string("div"));
  mElement.set("id", mId);

  // IMPORTANT: WebLayout owns positioning/mounting. Do not append here.
  // Use inline-block so bounding box + sizing works predictably.
  mElement["style"].set("display", std::string("inline-block"));
  mElement["style"].set("boxSizing", std::string("border-box"));

  ApplyText();
  ApplyStyles();
}


/// @brief Destructor: unmounts the element from the DOM and releases the reference.
WebTextbox::~WebTextbox()
{
  Unmount();
  mElement = val::null();
}


/// @brief Move constructor: transfers DOM ownership and all state from @p other.
/// @param other Source WebTextbox to move from.
WebTextbox::WebTextbox(WebTextbox&& other) noexcept
    : mText(std::move(other.mText)),
      mIsVisible(other.mIsVisible),
      mRequestedFontFamily(std::move(other.mRequestedFontFamily)),
      mFallbackFontFamily(std::move(other.mFallbackFontFamily)),
      mFontSizePx(other.mFontSizePx),
      mLineHeightPx(other.mLineHeightPx),
      mColor(std::move(other.mColor)),
      mBold(other.mBold),
      mItalic(other.mItalic),
      mTextAlign(std::move(other.mTextAlign)),
      mMaxWidthPx(other.mMaxWidthPx),
      mWrap(other.mWrap),
      mBackgroundColor(std::move(other.mBackgroundColor)),
      mAlign(other.mAlign)
{
  mId = std::move(other.mId);
  mElement = std::move(other.mElement);
  other.mElement = val::null();
  other.mIsVisible = false;
}


/// @brief Move assignment operator: transfers DOM ownership and all state from @p other.
/// @param other Source WebTextbox to move from.
/// @return Reference to this object.
WebTextbox& WebTextbox::operator=(WebTextbox&& other) noexcept
{
  if (this != &other)
  {
    Unmount();

    mText = std::move(other.mText);
    mIsVisible = other.mIsVisible;
    mRequestedFontFamily = std::move(other.mRequestedFontFamily);
    mFallbackFontFamily = std::move(other.mFallbackFontFamily);
    mFontSizePx = other.mFontSizePx;
    mLineHeightPx = other.mLineHeightPx;
    mColor = std::move(other.mColor);
    mBold = other.mBold;
    mItalic = other.mItalic;
    mTextAlign = std::move(other.mTextAlign);
    mMaxWidthPx = other.mMaxWidthPx;
    mWrap = other.mWrap;
    mBackgroundColor = std::move(other.mBackgroundColor);
    mElement = other.mElement;
    mId = std::move(other.mId);
    mAlign = other.mAlign;

    other.mElement = val::null();
    other.mIsVisible = false;
  }
  return *this;
}

/* ---------------- Text ---------------- */

/**
 * @brief Replaces the current text content and immediately updates the DOM element.
 *
 * @param text The new string to assign to the textbox.
 */
void WebTextbox::SetText(const std::string& text)
{
  mText = text;
  ApplyText();
}

/**
 * @brief Appends a string to the existing text content and updates the DOM element.
 *
 * @param text Additional text to append to the current content.
 */
void WebTextbox::AppendText(const std::string& text)
{
  mText += text;
  ApplyText();
}

/**
 * @brief Returns the current text content stored in the textbox.
 *
 * @return Copy of the current text string.
 */
std::string WebTextbox::GetText() const
{
  return mText;
}

/**
 * @brief Removes all text from the textbox and updates the DOM element.
 */
void WebTextbox::Clear()
{
  mText.clear();
  ApplyText();
}

/**
 * @brief Applies the current mText value to the DOM element's textContent.
 *
 * No-op if the DOM element has not yet been created.
 */
void WebTextbox::ApplyText()
{
  if (!mElement.isNull())
  {
    mElement.set("textContent", mText);
  }
}

/* -------------- Formatting -------------- */

/**
 * @brief Sets the primary font family and reapplies styles to the DOM.
 *
 * Emits a console warning if the requested font is not available in the browser.
 *
 * @param family The preferred CSS font-family name (e.g., "Arial").
 */
void WebTextbox::SetFontFamily(const std::string& family)
{
  mRequestedFontFamily = family;
  ApplyStyles();

  // Warning if computed font doesn't contain requested family
  val win = GetWindow();
  if (!mElement.isNull() && win.hasOwnProperty("getComputedStyle"))
  {
    val computed = win.call<val>("getComputedStyle", mElement);
    std::string applied = computed["fontFamily"].as<std::string>();
    if (!mRequestedFontFamily.empty() && applied.find(mRequestedFontFamily) == std::string::npos)
    {
      GetConsole().call<void>("warn", std::string("WebTextbox: Requested font '") + mRequestedFontFamily + "' not found; browser used: " + applied);
    }
  }
}

/**
 * @brief Sets the fallback font family appended after the primary in the font chain.
 *
 * @param fallback_family CSS fallback font-family name.
 */
void WebTextbox::SetFallbackFontFamily(const std::string& fallback_family)
{
  mFallbackFontFamily = fallback_family;
  ApplyStyles();
}

/**
 * @brief Sets the font size in pixels; must be > 0.
 *
 * Non-positive values are rejected in both debug (assert) and release
 * (early-return) builds to avoid invalid DOM state.
 *
 * @param size_px Font size in pixels, must be strictly greater than zero.
 */
void WebTextbox::SetFontSize(float size_px)
{
  assert(size_px > 0.0f);

    // Prevent invalid DOM state in release builds
  if (size_px <= 0.0f) 
  {
    return; 
  }
  mFontSizePx = size_px;
  ApplyStyles();
}

/**
 * @brief Sets the line height in pixels; must be > 0.
 *
 * Controls vertical spacing between lines. Non-positive values are
 * rejected to prevent invalid CSS line-height in the DOM.
 *
 * @param line_height_px Line height in pixels, must be greater than zero.
 */
void WebTextbox::SetLineHeight(float line_height_px)
{
  assert(line_height_px > 0.0f);

  // Prevent invalid DOM state in release builds
  if (line_height_px <= 0.0f) 
  {
    return; 
  }
  mLineHeightPx = line_height_px;
  ApplyStyles();
}

/**
 * @brief Updates the CSS text color and reapplies styles to the DOM element.
 *
 * @param css_color A valid CSS color string.
 */
void WebTextbox::SetColor(const std::string& css_color)
{
  mColor = css_color;
  ApplyStyles();
}

/**
 * @brief Enables or disables bold font weight and updates the DOM style.
 *
 * @param enabled True for bold text, false for normal weight.
 */
void WebTextbox::SetBold(bool enabled)
{
  mBold = enabled;
  ApplyStyles();
}

/**
 * @brief Enables or disables italic styling and updates the DOM style.
 *
 * @param enabled True to render text in italic, false for normal style.
 */
void WebTextbox::SetItalic(bool enabled)
{
  mItalic = enabled;
  ApplyStyles();
}

/**
 * @brief Sets the horizontal text alignment and reapplies styles to the DOM.
 *
 * @param alignment Desired text alignment (Left, Center, or Right).
 */
void WebTextbox::SetAlignment(TextAlign alignment)
{
    switch (alignment)
    {
        case TextAlign::Left:   mTextAlign = "left";   break;
        case TextAlign::Center: mTextAlign = "center"; break;
        case TextAlign::Right:  mTextAlign = "right";  break;
        default:
            assert(false && "Invalid TextAlign enum value");
            return;
    }

    ApplyStyles();
}

/**
 * @brief Sets the maximum width of the textbox in pixels; must be > 0.
 *
 * Non-positive values are rejected to prevent invalid CSS max-width values.
 *
 * @param width_px Maximum width in pixels, must be greater than zero.
 */
void WebTextbox::SetMaxWidth(float width_px)
{
  assert(width_px > 0.0f);

  // Prevent invalid DOM state in release builds
  if (width_px <= 0.0f)  
  {
    return;
  }
  mMaxWidthPx = width_px;
  ApplyStyles();
}

/**
 * @brief Controls whether text wraps (pre-wrap) or preserves whitespace without wrapping (pre).
 *
 * @param enabled True to enable wrapping, false to disable.
 */
void WebTextbox::SetWrap(bool enabled)
{
  mWrap = enabled;
  ApplyStyles();
}

/**
 * @brief Sets an explicit background color and applies it to the DOM element.
 *
 * Call ClearBackgroundColor() to revert to transparent.
 *
 * @param css_color A valid CSS color string for the background.
 */
void WebTextbox::SetBackgroundColor(const std::string& css_color)
{
  mBackgroundColor = css_color;
  ApplyStyles();
}

/**
 * @brief Clears any previously set background color, reverting to transparent.
 */
void WebTextbox::ClearBackgroundColor()
{
  mBackgroundColor.reset();
  ApplyStyles();
}

/**
 * ApplyStyles
 *
 * Applies the current style-related model state (font family, size, color,
 * line height, wrapping, max width, background color, visibility, and layout
 * alignment) to the underlying DOM element's style properties.
 *
 * This function is called internally by various setter methods and when
 * synchronizing the model to the view.
 */
void WebTextbox::ApplyStyles()
{
  if (mElement.isNull())
  {
    return;
  } 

  // Fonts
  std::string font_chain;
  if (!mRequestedFontFamily.empty())
  {
    font_chain = mRequestedFontFamily;
    if (!mFallbackFontFamily.empty())
    {
      font_chain += ", " + mFallbackFontFamily;
    }
  }
  else
  {
    font_chain = mFallbackFontFamily;
  }

  mElement["style"].set("fontFamily", font_chain);
  mElement["style"].set("fontSize", std::to_string(mFontSizePx) + "px");
  mElement["style"].set("color", mColor);
  mElement["style"].set("fontWeight", mBold ? "bold" : "normal");
  mElement["style"].set("fontStyle", mItalic ? "italic" : "normal");
  mElement["style"].set("textAlign", mTextAlign);

  // Line-height: if > 0 use px, else let browser choose
  if (mLineHeightPx > 0.0f)
  {
    mElement["style"].set("lineHeight", std::to_string(mLineHeightPx) + "px");
  }
  else
  {
    mElement["style"].set("lineHeight", std::string("normal"));
  }

  // Wrapping
  mElement["style"].set("whiteSpace", mWrap ? "pre-wrap" : "pre");

  // Max width
  if (mMaxWidthPx > 0.0f)
  {
    mElement["style"].set("maxWidth", std::to_string(mMaxWidthPx) + "px");
  }
  else
  {
    mElement["style"].set("maxWidth", std::string(""));
  }    
  // Background color
  if (mBackgroundColor.has_value())
  {
      mElement["style"].set("backgroundColor", *mBackgroundColor);
  }
  else
  {
      mElement["style"].set("backgroundColor", std::string("transparent"));
  }
  // Visibility
  mElement["style"].set("display", mIsVisible ? "inline-block" : "none");

  // Alignment (align-self) for flex/grid layouts in WebLayout
  ApplyAlignment(mAlign);
}

/**
 * ApplyAlignment
 *
 * Applies the given layout Alignment value to the DOM element's align-self
 * style property, used when the parent WebLayout is configured as a flex
 * or grid container.
 *
 * param:
 *    align - Layout alignment hint (start, center, end, or stretch).
 */
void WebTextbox::ApplyAlignment(Alignment align)
{
  if (mElement.isNull()) return;

  // Works when WebLayout uses flex/grid.
  switch (align)
  {
    case Alignment::Start:   mElement["style"].set("alignSelf", "flex-start"); break;
    case Alignment::Center:  mElement["style"].set("alignSelf", "center");     break;
    case Alignment::End:     mElement["style"].set("alignSelf", "flex-end");   break;
    case Alignment::Stretch: mElement["style"].set("alignSelf", "stretch");    break;
    default:                 mElement["style"].set("alignSelf", "flex-start"); break;
  }
}

/* -------------- Bounding box -------------- */

/**
 * @brief Returns the bounding box of the textbox’s DOM element in pixel units.
 *
 * Returns a zero-initialized RectPx if the element is not mounted.
 *
 * @return RectPx containing x, y, width, and height in pixels.
 */
WebTextbox::RectPx WebTextbox::GetBoundingBoxPx() const
{
  RectPx r_{};

  if (mElement.isNull()) return r_;

  // Must be mounted to get a real bounding box
  val parent_ = mElement["parentNode"];
  if (parent_.isNull() || parent_.isUndefined()) return r_;

  val rect_ = mElement.call<val>("getBoundingClientRect");
  r_.x = rect_["left"].as<double>();
  r_.y = rect_["top"].as<double>();
  r_.w = rect_["width"].as<double>();
  r_.h = rect_["height"].as<double>();
  return r_;
}

/**
 * @brief Returns the width component of the textbox’s bounding box.
 *
 * @return Width in pixels, or 0 if the element is not mounted.
 */
double WebTextbox::GetWidthPx() const  
{ 
  return GetBoundingBoxPx().w;
}

/**
 * @brief Returns the height component of the textbox’s bounding box.
 *
 * @return Height in pixels, or 0 if the element is not mounted.
 */
double WebTextbox::GetHeightPx() const 
{ 
  return GetBoundingBoxPx().h; 
}

/* -------------- Visibility -------------- */

/**
 * @brief Makes the textbox visible by setting the visibility flag and updating the DOM.
 */
void WebTextbox::Show()
{
  mIsVisible = true;
  ApplyStyles();
}

/**
 * @brief Hides the textbox by clearing the visibility flag and updating the DOM.
 */
void WebTextbox::Hide()
{
  mIsVisible = false;
  ApplyStyles();
}

/**
 * @brief Returns whether the textbox is currently marked as visible.
 *
 * @return True if the textbox should be shown; false otherwise.
 */
bool WebTextbox::IsVisible() const
{
  return mIsVisible;
}

/* -------------- IDomElement -------------- */

/**
 * MountToLayout
 *
 * Mounts the textbox into a parent WebLayout by inserting its DOM element
 * into the parent's DOM container. If the element is already mounted
 * elsewhere, it is first unmounted.
 *
 * param:
 *    parent - The WebLayout this textbox should be attached to.
 *    align  - Alignment preference relative to the layout container.
 */
void WebTextbox::MountToLayout(WebLayout& parent, Alignment align)
{
  mAlign = align;

  // If already mounted somewhere else, unmount first.
  Unmount();

  parent.AddElement(this, align);
}

/**
 * @brief Synchronizes internal model values (text and style) with the DOM element.
 *
 * Called automatically after mounting.
 */
void WebTextbox::SyncFromModel()
{
  ApplyText();
  ApplyStyles();
}

// -------- ICanvasElement overrides --------

/// @brief Stores the canvas-space position used by Draw().
void WebTextbox::SetCanvasPosition(float x, float y)
{
  mCanvasX = x;
  mCanvasY = y;
}

/// @brief Draws the textbox text onto @p canvas at the position set by
///        SetCanvasPosition(), using the current color and font-size settings.
void WebTextbox::Draw(WebCanvas& canvas)
{
  if (mText.empty()) {
    return;
  }
  canvas.DrawText(mCanvasX, mCanvasY, mText, mColor, mFontSizePx);
}

}
