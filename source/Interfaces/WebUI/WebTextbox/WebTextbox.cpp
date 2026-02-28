/**
 * WebTextbox.cpp
 * Implementation of the WebTextbox class for styled text elements in the WebUI.
 *
 * This file contains the DOM interaction logic, style application, and
 * model view synchronization used by the WebTextbox component within the
 * CSE 498 WebUI subsystem. Emscripten’s embind APIs are used to interface
 * with underlying JavaScript DOM objects.
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

#include "WebTextbox.hpp"

#include <cassert>
#include <utility>
#include <emscripten/val.h>

using emscripten::val;

namespace cse498 {

int WebTextbox::mNextIdCounter = 1;

/** 
  * -------------- Helpers --------------
  * These helper functions centralize access to global JS objects.
  * They also avoid repeated calls to val::global("document") etc. which may have some overhead.
  */
static val GetDocument() 
{ 
  return val::global("document"); 
}
static val GetWindow()   
{ 
  return val::global("window"); 
}
static val GetConsole()  
{ 
  return val::global("console"); 
}

/**
 * Constuctor / Destructor / Move semantics
 */
WebTextbox::WebTextbox()
    : WebTextbox(std::string(""))
{
}
// Constructor creates element with correct text and styles
WebTextbox::WebTextbox(const std::string& initial_text)
    : mText(initial_text),
      mElement(val::null())
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


// Destructor unmounts from DOM if needed and releases element reference.
WebTextbox::~WebTextbox()
{
  Unmount();
  mElement = val::null();
}


// Move constructor and move assignment transfer ownership of the DOM element and state.
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
      mElement(other.mElement),
      mId(std::move(other.mId)),
      mAlign(other.mAlign)
{
  other.mElement = val::null();
  other.mIsVisible = false;
}


// Move assignment operator
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
 * SetText
 * 
 * Replaces the current text stored in mText and immediately updates the
 * associated DOM element via ApplyText().
 *
 * param:
 *    text - The new string to assign to the textbox.
 */
void WebTextbox::SetText(const std::string& text)
{
  mText = text;
  ApplyText();
}

/**
 * AppendText
 *
 * Appends the provided string to the existing mText value and updates the
 * DOM representation by calling ApplyText().
 *
 * param:
 *    text - Additional text to append to the current content.
 */
void WebTextbox::AppendText(const std::string& text)
{
  mText += text;
  ApplyText();
}

/**
 * AppendText
 *
 * Appends the provided string to the existing mText value and updates the
 * DOM representation by calling ApplyText().
 *
 * param:
 *    text - Additional text to append to the current content.
 */
std::string WebTextbox::GetText() const
{
  return mText;
}

/**
 * Clear
 *
 * Removes all text from mText and updates the DOM through ApplyText().
 */
void WebTextbox::Clear()
{
  mText.clear();
  ApplyText();
}

/**
 * Clear
 *
 * Removes all text from mText and updates the DOM through ApplyText().
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
 * SetFontFamily
 *
 * Updates the primary font family used when rendering the textbox text.
 * Also reapplies styles to propagate the change to the DOM and emits a
 * warning to the console if the requested font is not actually applied.
 *
 * param:
 *    family - The preferred CSS font-family name (e.g., "Arial").
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
 * SetFallbackFontFamily
 *
 * Sets a fallback font family that will be appended to the primary font
 * family chain used by the textbox. The updated font chain is then applied
 * to the DOM via ApplyStyles().
 *
 * param:
 *    fallback_family - CSS fallback font-family
 */
void WebTextbox::SetFallbackFontFamily(const std::string& fallback_family)
{
  mFallbackFontFamily = fallback_family;
  ApplyStyles();
}

/**
 * SetFontSize
 *
 * Sets the font size (in pixels) used to render the textbox text. Invalid
 * non-positive values are rejected in both debug (assert) and release
 * (early return) builds to avoid invalid DOM state.
 *
 * param:
 *    size_px - Font size in pixels, must be strictly greater than zero.
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
 * SetLineHeight
 *
 * Sets the line height (in pixels) used for the textbox text. This
 * controls vertical spacing between lines. Non-positive values are
 * rejected to prevent invalid CSS line-height in the DOM.
 *
 * param:
 *    line_height_px - Line height in pixels, must be greater than zero.
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
 * SetColor
 *
 * Updates the CSS text color for the textbox and reapplies styles to
 * the DOM element.
 *
 * param:
 *    css_color - A valid CSS color string
 */
void WebTextbox::SetColor(const std::string& css_color)
{
  mColor = css_color;
  ApplyStyles();
}

/**
 * SetBold
 *
 * Enables or disables bold font weight for the textbox text and updates
 * the DOM style accordingly.
 *
 * param:
 *    enabled - True to set bold text, false for normal weight.
 */
void WebTextbox::SetBold(bool enabled)
{
  mBold = enabled;
  ApplyStyles();
}

/**
 * SetItalic
 *
 * Enables or disables italic styling for the textbox text and updates
 * the DOM style accordingly.
 *
 * param:
 *    enabled - True to render text in italic, false for normal style.
 */
void WebTextbox::SetItalic(bool enabled)
{
  mItalic = enabled;
  ApplyStyles();
}

/**
 * SetAlignment
 *
 * Sets the horizontal text alignment within the textbox using a strongly
 * typed TextAlign enum, then reapplies styles to the DOM.
 *
 * param:
 *    alignment - Desired text alignment (left, center, or right).
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
 * SetMaxWidth
 *
 * Sets the maximum width of the textbox (in pixels). Non-positive values
 * are rejected to prevent invalid CSS max-width values.
 *
 * param:
 *    width_px - Maximum width in pixels, must be greater than zero.
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
 * SetWrap
 *
 * Controls whether the textbox text wraps across multiple lines or
 * preserves whitespace and line breaks without wrapping.
 *
 * param:
 *    enabled - True to enable wrapping (pre-wrap), false for no wrapping (pre).
 */
void WebTextbox::SetWrap(bool enabled)
{
  mWrap = enabled;
  ApplyStyles();
}

/**
 * SetBackgroundColor
 *
 * Sets an explicit background color for the textbox and applies it to
 * the DOM element. A later call to ClearBackgroundColor() will remove
 * this and revert to transparent.
 *
 * param:
 *    css_color - A valid CSS color string for the background.
 */
void WebTextbox::SetBackgroundColor(const std::string& css_color)
{
  mBackgroundColor = css_color;
  ApplyStyles();
}

/**
 * ClearBackgroundColor
 *
 * Clears any previously set background color from the textbox model,
 * causing the DOM element to use a transparent background.
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
 * GetBoundingBoxPx
 *
 * Returns the bounding box of the textbox’s DOM element in pixel units.
 * If the element is not mounted or the DOM node is missing, a zero-initialized
 * RectPx is returned.
 *
 * return:
 *    Struct containing x, y, width, and height in pixels.
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
 * GetWidthPx
 *
 * Returns the width component of the textbox’s bounding box.
 *
 * return:
 *    Width in pixels, or 0 if the element is not mounted.
 */
double WebTextbox::GetWidthPx() const  
{ 
  return GetBoundingBoxPx().w;
}

/**
 * GetHeightPx
 *
 * Returns the height component of the textbox’s bounding box.
 *
 * return:
 *    Height in pixels, or 0 if the element is not mounted.
 */
double WebTextbox::GetHeightPx() const 
{ 
  return GetBoundingBoxPx().h; 
}

/* -------------- Visibility -------------- */

/**
 * Show
 *
 * Makes the textbox visible by setting the internal visibility flag and
 * updating the DOM display style.
 */
void WebTextbox::Show()
{
  mIsVisible = true;
  ApplyStyles();
}

/**
 * Hide
 *
 * Hides the textbox by disabling visibility and updating the DOM display style.
 */
void WebTextbox::Hide()
{
  mIsVisible = false;
  ApplyStyles();
}

/**
 * IsVisible
 *
 * Indicates whether the textbox is currently marked as visible.
 *
 * return:
 *    True if the textbox should be shown; false otherwise.
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

  // Find parents DOM element by id and append ourselves.
  // This avoids depending on WebLayout internals and matches WebImage Id() pattern.
  val doc_ = GetDocument();
  val parentEl_ = doc_.call<val>("getElementById", parent.Id());

  // If parent isnt in DOM yet, this will be null layout should ensure mount order
  if (parentEl_.isNull() || parentEl_.isUndefined())
  {
    GetConsole().call<void>("warn",std::string("WebTextbox: mountToLayout failed; parent element not found: ") + parent.Id());
    return;
  }

  // If already mounted somewhere else, unmount first.
  Unmount();

  parentEl_.call<void>("appendChild", mElement);
  SyncFromModel();
}

/**
 * Unmount
 *
 * Removes the textbox's DOM element from its current parent container,
 * if it is currently mounted.
 */
void WebTextbox::Unmount()
{
  if (mElement.isNull())
  {
     return;
  }

  val parent_ = mElement["parentNode"];
  if (!parent_.isNull() && !parent_.isUndefined())
  {
    parent_.call<void>("removeChild", mElement);
  }
}

/**
 * SyncFromModel
 *
 * Synchronizes internal model values (text + style properties)
 * with the DOM element. Called automatically after mounting.
 */
void WebTextbox::SyncFromModel()
{
  ApplyText();
  ApplyStyles();
}

/**
 * Id
 *
 * Returns the unique string identifier assigned to this textbox’s
 * DOM element. Useful for locating the element in DOM operations.
 *
 * return:
 *    Const reference to the unique element ID string.
 */
const std::string& WebTextbox::Id() const
{
  return mId;
}
}
