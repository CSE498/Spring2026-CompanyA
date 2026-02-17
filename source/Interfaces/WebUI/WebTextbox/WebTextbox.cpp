#include "WebTextbox.hpp"

#include <cassert>
#include <utility>
#include <emscripten/val.h>

using emscripten::val;

int WebTextbox::next_id_counter_ = 1;

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
    : text_(initial_text),
      element_(val::null())
{
  // unique id like WebImage
  id_ = "webtextbox-" + std::to_string(next_id_counter_++);

  val doc_ = GetDocument();
  element_ = doc_.call<val>("createElement", std::string("div"));
  element_.set("id", id_);

  // IMPORTANT: WebLayout owns positioning/mounting. Do not append here.
  // Use inline-block so bounding box + sizing works predictably.
  element_["style"].set("display", std::string("inline-block"));
  element_["style"].set("boxSizing", std::string("border-box"));

  ApplyText();
  ApplyStyles();
}


// Destructor unmounts from DOM if needed and releases element reference.
WebTextbox::~WebTextbox()
{
  unmount();
  element_ = val::null();
}


// Move constructor and move assignment transfer ownership of the DOM element and state.
WebTextbox::WebTextbox(WebTextbox&& other) noexcept
    : text_(std::move(other.text_)),
      is_visible_(other.is_visible_),
      requested_font_family_(std::move(other.requested_font_family_)),
      fallback_font_family_(std::move(other.fallback_font_family_)),
      font_size_px_(other.font_size_px_),
      line_height_px_(other.line_height_px_),
      color_(std::move(other.color_)),
      bold_(other.bold_),
      italic_(other.italic_),
      text_align_(std::move(other.text_align_)),
      max_width_px_(other.max_width_px_),
      wrap_(other.wrap_),
      background_color_(std::move(other.background_color_)),
      element_(other.element_),
      id_(std::move(other.id_)),
      align_(other.align_)
{
  other.element_ = val::null();
  other.is_visible_ = false;
}


// Move assignment operator
WebTextbox& WebTextbox::operator=(WebTextbox&& other) noexcept
{
  if (this != &other)
  {
    unmount();

    text_ = std::move(other.text_);
    is_visible_ = other.is_visible_;
    requested_font_family_ = std::move(other.requested_font_family_);
    fallback_font_family_ = std::move(other.fallback_font_family_);
    font_size_px_ = other.font_size_px_;
    line_height_px_ = other.line_height_px_;
    color_ = std::move(other.color_);
    bold_ = other.bold_;
    italic_ = other.italic_;
    text_align_ = std::move(other.text_align_);
    max_width_px_ = other.max_width_px_;
    wrap_ = other.wrap_;
    background_color_ = std::move(other.background_color_);
    element_ = other.element_;
    id_ = std::move(other.id_);
    align_ = other.align_;

    other.element_ = val::null();
    other.is_visible_ = false;
  }
  return *this;
}

/* ---------------- Text ---------------- */

void WebTextbox::SetText(const std::string& text)
{
  text_ = text;
  ApplyText();
}

void WebTextbox::AppendText(const std::string& text)
{
  text_ += text;
  ApplyText();
}

std::string WebTextbox::GetText() const
{
  return text_;
}

void WebTextbox::Clear()
{
  text_.clear();
  ApplyText();
}

void WebTextbox::ApplyText()
{
  if (!element_.isNull())
  {
    element_.set("textContent", text_);
  }
}

/* -------------- Formatting -------------- */

void WebTextbox::SetFontFamily(const std::string& family)
{
  requested_font_family_ = family;
  ApplyStyles();

  // Warning if computed font doesn't contain requested family
  val win = GetWindow();
  if (!element_.isNull() && win.hasOwnProperty("getComputedStyle"))
  {
    val computed = win.call<val>("getComputedStyle", element_);
    std::string applied = computed["fontFamily"].as<std::string>();
    if (!requested_font_family_.empty() && applied.find(requested_font_family_) == std::string::npos)
    {
      GetConsole().call<void>("warn", std::string("WebTextbox: Requested font '") + requested_font_family_ + "' not found; browser used: " + applied);
    }
  }
}

void WebTextbox::SetFallbackFontFamily(const std::string& fallback_family)
{
  fallback_font_family_ = fallback_family;
  ApplyStyles();
}

void WebTextbox::SetFontSize(float size_px)
{
  assert(size_px > 0.0f);
  font_size_px_ = size_px;
  ApplyStyles();
}

void WebTextbox::SetLineHeight(float line_height_px)
{
  assert(line_height_px > 0.0f);
  line_height_px_ = line_height_px;
  ApplyStyles();
}

void WebTextbox::SetColor(const std::string& css_color)
{
  color_ = css_color;
  ApplyStyles();
}

void WebTextbox::SetBold(bool enabled)
{
  bold_ = enabled;
  ApplyStyles();
}

void WebTextbox::SetItalic(bool enabled)
{
  italic_ = enabled;
  ApplyStyles();
}

void WebTextbox::SetAlignment(const std::string& alignment)
{
  assert(alignment == "left" || alignment == "center" || alignment == "right");
  text_align_ = alignment;
  ApplyStyles();
}

void WebTextbox::SetMaxWidth(float width_px)
{
  assert(width_px > 0.0f);
  max_width_px_ = width_px;
  ApplyStyles();
}

void WebTextbox::SetWrap(bool enabled)
{
  wrap_ = enabled;
  ApplyStyles();
}

void WebTextbox::SetBackgroundColor(const std::string& css_color)
{
  background_color_ = css_color;
  ApplyStyles();
}

void WebTextbox::ClearBackgroundColor()
{
  background_color_.clear();
  ApplyStyles();
}

void WebTextbox::ApplyStyles()
{
  if (element_.isNull())
  {
    return;
  } 

  // Fonts
  std::string font_chain;
  if (!requested_font_family_.empty())
  {
    font_chain = requested_font_family_;
    if (!fallback_font_family_.empty())
    {
      font_chain += ", " + fallback_font_family_;
    }
  }
  else
  {
    font_chain = fallback_font_family_;
  }

  element_["style"].set("fontFamily", font_chain);
  element_["style"].set("fontSize", std::to_string(font_size_px_) + "px");
  element_["style"].set("color", color_);
  element_["style"].set("fontWeight", bold_ ? "bold" : "normal");
  element_["style"].set("fontStyle", italic_ ? "italic" : "normal");
  element_["style"].set("textAlign", text_align_);

  // Line-height: if > 0 use px, else let browser choose
  if (line_height_px_ > 0.0f)
  {
    element_["style"].set("lineHeight", std::to_string(line_height_px_) + "px");
  }
  else
  {
    element_["style"].set("lineHeight", std::string("normal"));
  }

  // Wrapping
  element_["style"].set("whiteSpace", wrap_ ? "pre-wrap" : "pre");

  // Max width
  if (max_width_px_ > 0.0f)
  {
    element_["style"].set("maxWidth", std::to_string(max_width_px_) + "px");
  }
  else
  {
    element_["style"].set("maxWidth", std::string(""));
  }    
  // Background color
  element_["style"].set("backgroundColor", background_color_.empty() ? "transparent" : background_color_);

  // Visibility
  element_["style"].set("display", is_visible_ ? "inline-block" : "none");

  // Alignment (align-self) for flex/grid layouts in WebLayout
  ApplyAlignment(align_);
}

void WebTextbox::ApplyAlignment(Alignment align)
{
  if (element_.isNull()) return;

  // Works when WebLayout uses flex/grid.
  switch (align)
  {
    case Alignment::Start:   element_["style"].set("alignSelf", "flex-start"); break;
    case Alignment::Center:  element_["style"].set("alignSelf", "center");     break;
    case Alignment::End:     element_["style"].set("alignSelf", "flex-end");   break;
    case Alignment::Stretch: element_["style"].set("alignSelf", "stretch");    break;
    default:                 element_["style"].set("alignSelf", "flex-start"); break;
  }
}

/* -------------- Bounding box -------------- */

WebTextbox::RectPx WebTextbox::GetBoundingBoxPx() const
{
  RectPx r_{};

  if (element_.isNull()) return r_;

  // Must be mounted to get a real bounding box
  val parent_ = element_["parentNode"];
  if (parent_.isNull() || parent_.isUndefined()) return r_;

  val rect_ = element_.call<val>("getBoundingClientRect");
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
  is_visible_ = true;
  ApplyStyles();
}

void WebTextbox::Hide()
{
  is_visible_ = false;
  ApplyStyles();
}

bool WebTextbox::IsVisible() const
{
  return is_visible_;
}

/* -------------- IDomElement -------------- */

void WebTextbox::mountToLayout(WebLayout& parent, Alignment align)
{
  align_ = align;

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
  unmount();

  parentEl_.call<void>("appendChild", element_);
  syncFromModel();
}

void WebTextbox::unmount()
{
  if (element_.isNull())
  {
     return;
  }

  val parent_ = element_["parentNode"];
  if (!parent_.isNull() && !parent_.isUndefined())
  {
    parent_.call<void>("removeChild", element_);
  }
}

void WebTextbox::syncFromModel()
{
  ApplyText();
  ApplyStyles();
}

const std::string& WebTextbox::Id() const
{
  return id_;
}
