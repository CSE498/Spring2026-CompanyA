#include "WebTextbox.hpp"

#include <cassert>
#include <emscripten/val.h>

using emscripten::val;

// Helper to get the browser's document object
static val GetDocument() 
{
  return val::global("document");
}

// Construction
WebTextbox::WebTextbox(const std::string& initial_text)
    : text_(initial_text),
      is_visible_(true),
      element_(val::null()) {
  val doc = GetDocument();

  // Create a <div> for text (could also be <span> depending on use-case)
  element_ = doc.call<val>("createElement", std::string("div"));

  // Basic styling
  element_["style"].set("position", std::string("absolute"));
  element_["style"].set("whiteSpace", std::string("pre-wrap"));  // preserve newlines

  // Set initial text
  ApplyText();

  // Attach to document body
  doc["body"].call<void>("appendChild", element_);
}


// Destruction
WebTextbox::~WebTextbox() 
{
  if (!element_.isNull()) 
  {
    element_["parentNode"].call<void>("removeChild", element_);
    element_ = val::null();
  }
}


// Move Constructor
WebTextbox::WebTextbox(WebTextbox&& other) noexcept
    : text_(std::move(other.text_)),
      is_visible_(other.is_visible_),
      element_(other.element_) {
  other.element_ = val::null();
  other.is_visible_ = false;
}


// Move Assignment
WebTextbox& WebTextbox::operator=(WebTextbox&& other) noexcept 
{
  if (this != &other) 
  {
    if (!element_.isNull()) 
    {
      element_["parentNode"].call<void>("removeChild", element_);
    }

    text_ = std::move(other.text_);
    is_visible_ = other.is_visible_;
    element_ = other.element_;

    other.element_ = val::null();
    other.is_visible_ = false;
  }
  return *this;
}


/*Core Text Operations*/


// Sets the entire text content of the textbox
void WebTextbox::SetText(const std::string& text) 
{
  text_ = text;
  ApplyText();
}


// Appends additional text to the existing content
void WebTextbox::AppendText(const std::string& text) 
{
  text_ += text;
  ApplyText();
}


// Retrieves the current text content of the textbox
std::string WebTextbox::GetText() const 
{
  return text_;
}


// Clears all text from the textbox
void WebTextbox::Clear() 
{
  text_.clear();
  ApplyText();
}


// Private helper to update the DOM element's text content based on the current text_ member
void WebTextbox::ApplyText() 
{
  if (!element_.isNull()) 
  {
    element_.set("textContent", text_);
  }
}


/*Formatting*/


// Sets the font family
void WebTextbox::SetFontFamily(const std::string& family) 
{
  if (!element_.isNull()) 
  {
    element_["style"].set("fontFamily", family);
  }
}


// Sets the font size in pixels
void WebTextbox::SetFontSize(float size_px) 
{
  assert(size_px > 0.0f && "SetFontSize: size must be positive");
  if (!element_.isNull()) 
  {
    element_["style"].set("fontSize", std::to_string(size_px) + "px");
  }
}


// Sets the text color using any valid CSS color string
void WebTextbox::SetColor(const std::string& css_color) 
{
  if (!element_.isNull()) 
  {
    element_["style"].set("color", css_color);
  }
}


// Enables or disables bold text
void WebTextbox::SetBold(bool enabled) 
{
  if (!element_.isNull()) 
  {
    element_["style"].set("fontWeight", enabled ? "bold" : "normal");
  }
}


// Enables or disables italic text
void WebTextbox::SetItalic(bool enabled) 
{
  if (!element_.isNull()) 
  {
    element_["style"].set("fontStyle", enabled ? "italic" : "normal");
  }
}


// Sets text alignment: "left", "center", or "right"
void WebTextbox::SetAlignment(const std::string& alignment) 
{
    assert((alignment == "left" || alignment == "center" || alignment == "right") &&
             "SetAlignment: alignment must be 'left', 'center', or 'right'");
  if (!element_.isNull()) 
  {
    element_["style"].set("textAlign", alignment);
  }
}


// Sets the maximum width of the textbox in pixels
void WebTextbox::SetMaxWidth(float width_px) 
{
  assert(width_px > 0.0f && "SetMaxWidth: width must be positive");
  if (!element_.isNull()) 
  {
    element_["style"].set("maxWidth", std::to_string(width_px) + "px");
  }
}



/* Layout */


// Sets the position of the textbox on the page
void WebTextbox::SetPosition(int x, int y) 
{
  if (!element_.isNull()) 
  {
    element_["style"].set("left", std::to_string(x) + "px");
    element_["style"].set("top", std::to_string(y) + "px");
  }
}


/* Visibility */ 


// Shows the textbox
void WebTextbox::Show() 
{
  is_visible_ = true;
  if (!element_.isNull()) 
  {
    element_["style"].set("display", std::string(""));
  }
}


// Hides the textbox
void WebTextbox::Hide() 
{
  is_visible_ = false;
  if (!element_.isNull()) 
  {
    element_["style"].set("display", std::string("none"));
  }
}


// Checks if the textbox is currently visible
bool WebTextbox::IsVisible() const 
{
  return is_visible_;
}