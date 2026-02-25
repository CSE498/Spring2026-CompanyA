#include "WebTextbox.hpp"

#include <cassert>
#include <utility>
#include <emscripten/val.h>

using emscripten::val;

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

void WebTextbox::SetText(const std::string& text)
{
  mText = text;
  ApplyText();
}

void WebTextbox::AppendText(const std::string& text)
{
  mText += text;
  ApplyText();
}

std::string WebTextbox::GetText() const
{
  return mText;
}

void WebTextbox::Clear()
{
  mText.clear();
  ApplyText();
}

void WebTextbox::ApplyText()
{
  if (!mElement.isNull())
  {
    mElement.set("textContent", mText);
  }
}

/* -------------- Formatting -------------- */

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

void WebTextbox::SetFallbackFontFamily(const std::string& fallback_family)
{
  mFallbackFontFamily = fallback_family;
  ApplyStyles();
}

void WebTextbox::SetFontSize(float size_px)
{
  assert(size_px > 0.0f);
  mFontSizePx = size_px;
  ApplyStyles();
}

void WebTextbox::SetLineHeight(float line_height_px)
{
  assert(line_height_px > 0.0f);
  mLineHeightPx = line_height_px;
  ApplyStyles();
}

void WebTextbox::SetColor(const std::string& css_color)
{
  mColor = css_color;
  ApplyStyles();
}

void WebTextbox::SetBold(bool enabled)
{
  mBold = enabled;
  ApplyStyles();
}

void WebTextbox::SetItalic(bool enabled)
{
  mItalic = enabled;
  ApplyStyles();
}

void WebTextbox::SetAlignment(const std::string& alignment)
{
  assert(alignment == "left" || alignment == "center" || alignment == "right");
  mTextAlign = alignment;
  ApplyStyles();
}

void WebTextbox::SetMaxWidth(float width_px)
{
  assert(width_px > 0.0f);
  mMaxWidthPx = width_px;
  ApplyStyles();
}

void WebTextbox::SetWrap(bool enabled)
{
  mWrap = enabled;
  ApplyStyles();
}

void WebTextbox::SetBackgroundColor(const std::string& css_color)
{
  mBackgroundColor = css_color;
  ApplyStyles();
}

void WebTextbox::ClearBackgroundColor()
{
  mBackgroundColor.clear();
  ApplyStyles();
}

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
  mElement["style"].set("backgroundColor", mBackgroundColor.empty() ? "transparent" : mBackgroundColor);

  // Visibility
  mElement["style"].set("display", mIsVisible ? "inline-block" : "none");

  // Alignment (align-self) for flex/grid layouts in WebLayout
  ApplyAlignment(mAlign);
}

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

double WebTextbox::GetWidthPx() const  
{ 
  return GetBoundingBoxPx().w;
}
double WebTextbox::GetHeightPx() const 
{ 
  return GetBoundingBoxPx().h; 
}

/* -------------- Visibility -------------- */

void WebTextbox::Show()
{
  mIsVisible = true;
  ApplyStyles();
}

void WebTextbox::Hide()
{
  mIsVisible = false;
  ApplyStyles();
}

bool WebTextbox::IsVisible() const
{
  return mIsVisible;
}

/* -------------- IDomElement -------------- */

void WebTextbox::MountToLayout(WebLayout& parent, Alignment align)
{
  mAlign = align;

  // Find parent's DOM element by id and append ourselves.
  // This avoids depending on WebLayout internals and matches WebImage Id() pattern.
  val doc_ = GetDocument();
  val parentEl_ = doc_.call<val>("getElementById", parent.Id());

  // If parent isn't in DOM yet, this will be null — layout should ensure mount order.
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

void WebTextbox::SyncFromModel()
{
  ApplyText();
  ApplyStyles();
}

const std::string& WebTextbox::Id() const
{
  return mId;
}
