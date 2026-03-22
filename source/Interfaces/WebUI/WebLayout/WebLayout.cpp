/**
 * @file WebLayout.cpp
 * @brief Implementation of WebLayout DOM container and layout engine.
 *
 * Contains the DOM interaction logic for creating, configuring, and
 * destroying a root \<div\> element, as well as applying Flexbox, CSS Grid,
 * or Free (absolute/relative) positioning to child IDomElement objects.
 * Requires Emscripten; the entire file is conditionally compiled with
 * \c __EMSCRIPTEN__.
 *
 */

#ifdef __EMSCRIPTEN__

#include "WebLayout.hpp"
#include "../WebUtils.hpp"

#include <string>

using emscripten::val;

namespace cse498 {

int WebLayout::mNextIdCounter = 1;

/// Helper function to convert a pixel value to a CSS string.
/// @param v Pixel value (non-negative)
/// @return CSS string like "10px" or empty string if v < 0
static std::string px(int v) noexcept {
  if (v < 0) return std::string();
  return std::to_string(v) + "px";
}

/// @brief Constructs a WebLayout, creating or adopting a root DOM element.
/// @param rootId Optional ID; if an element with this ID exists in the DOM it
///               is adopted as root, otherwise a new \<div\> is created and
///               appended to \<body\>.
WebLayout::WebLayout(const std::string& rootId) noexcept {
  mDocument = val::global("document");
  if (!rootId.empty()) {
    val found = mDocument.call<val>("getElementById", rootId);
    if (!found.isNull() && !found.isUndefined()) {
      mElement = found;
    } else {
      // create a container div with the requested id and append to body
      mElement = mDocument.call<val>("createElement", std::string("div"));
      mElement.set("id", rootId);
    }
    mId = rootId;
    mElement["style"].set("boxSizing", std::string("border-box"));
    return;
  }

  // no id passed: create a container div and append to body with generated id
  mId = "weblayout-" + std::to_string(mNextIdCounter++);
  mElement = mDocument.call<val>("createElement", std::string("div"));
  mElement.set("id", mId);
  mElement["style"].set("boxSizing", std::string("border-box"));
  assert(!mElement.isNull() && !mElement.isUndefined());
  assert(mElement["id"].as<std::string>() == mId);
}

/// @brief Destructor: unmounts all children and removes the root element from the DOM.
WebLayout::~WebLayout() noexcept {
  // unmount children
  Clear();

  // remove the root element from DOM if present
  Unmount();
  mDocument = val::undefined();
  mElement = val::undefined();
}

/// @brief Sets the layout type (Free, Horizontal, Vertical, Grid).
/// @param t Desired LayoutType value.
void WebLayout::SetLayoutType(LayoutType t) noexcept { mType = t; }

/// @brief Sets the main-axis justification for flex/grid child elements.
/// @param j Desired Justification value.
void WebLayout::SetJustification(Justification j) noexcept {
  mJustification = j;
}

/// @brief Sets the cross-axis alignment for all child elements.
/// @param a Desired Alignment value.
void WebLayout::SetAlignItems(Alignment a) noexcept { mAlignItems = a; }

/// @brief Sets the gap/spacing between child elements.
/// @param pxv Spacing in pixels; must be >= 0.
void WebLayout::SetSpacing(int pxv) noexcept {
  if (pxv < 0) return;
  mSpacing = pxv;
}

/// @brief Adds a child element to this layout and appends it to the DOM.
/// @param elem  Pointer to the IDomElement to add; must not be null.
/// @param align Alignment for this child within the layout.
/// @return True if successfully added; false if elem is null or has no DOM id.
bool WebLayout::AddElement(IDomElement* elem, Alignment align) noexcept {
  if (!elem) return false;
  const ElementID& id = elem->Id();
  if (id.empty()) return false;

  auto element = elem->GetElement();

  if (element.isNull() || element.isUndefined()) {
    GetConsole().call<void>("warn", "element with Id: " + elem->Id() + " is undefined");
    return false;
  }

  // attach now; Apply() will re-check parent when re-applying
  // if element already exists in this layout, this will just move it to the end
  mElement.call<void>("appendChild", elem->GetElement());
  elem->SetParent(this);

  // if element already exists in this layout, don't add it to mChildren again
  if (std::find(mChildren.begin(), mChildren.end(), elem) != mChildren.end()) return true;

  mChildren.push_back(elem);
  mParams[elem] = align;
  return true;
}

/// @brief Removes a child element from this layout and the DOM.
/// @param elem Pointer to the child element to remove.
/// @return True if found and removed; false if not found.
bool WebLayout::RemoveElement(IDomElement* elem) noexcept {
  if (!elem) return false;
  auto it = std::find(mChildren.begin(), mChildren.end(), elem);
  if (it == mChildren.end()) return false;
  mChildren.erase(it);
  mParams.erase(elem);

  // remove DOM element from this layout
  IDomElement::RemoveChild(elem);
  return true;
}

/// @brief Updates the per-child alignment stored in this layout.
/// @param elem Pointer to the child element to update.
/// @param a    New Alignment value for that child.
void WebLayout::SetAlignment(IDomElement* elem, Alignment a) noexcept {
  if (!elem) return;
  auto it = mParams.find(elem);
  if (it == mParams.end()) return;
  it->second = a;
}

// ===== Styling Methods =====

/// @brief Sets the background color of the layout container.
/// @param color CSS color string (e.g., "#ffffff", "red").
void WebLayout::SetBackgroundColor(const std::string& color) noexcept {
  mBackgroundColor = color;
}

/// @brief Sets the border color of the layout container.
/// @param color CSS color string.
void WebLayout::SetBorderColor(const std::string& color) noexcept {
  mBorderColor = color;
}

/// @brief Sets the border width in pixels; must be >= 0.
/// @param width Border width in pixels.
void WebLayout::SetBorderWidth(int width) noexcept {
  if (width < 0) {
    GetConsole().call<void>("warn", "Invalid Border Width value: " + std::to_string(width));
    return;
  }
  mBorderWidth = width;
}

/// @brief Sets the border radius for rounded corners; must be >= 0.
/// @param radius Border radius in pixels.
void WebLayout::SetBorderRadius(int radius) noexcept {
  if (radius < 0) {
    GetConsole().call<void>("warn", "Invalid Border Radius value: " + std::to_string(radius));
    return;
  }
  mBorderRadius = radius;
}

/// @brief Sets the padding of the layout container; must be >= 0.
/// @param padding Padding size in pixels.
void WebLayout::SetPadding(int padding) noexcept {
  if (padding < 0) {
    GetConsole().call<void>("warn", "Invalid Padding value: " + std::to_string(padding));
    return;
  }
  mPadding = padding;
}

/// @brief Sets the margin of the layout container; must be >= 0.
/// @param margin Margin size in pixels.
void WebLayout::SetMargin(int margin) noexcept {
  if (margin < 0) {
    GetConsole().call<void>("warn", "Invalid Margin value: " + std::to_string(margin));
    return;
  }
  mMargin = margin;
}

/// @brief Sets the width of the layout container; must be > 0.
/// @param width Width in pixels.
void WebLayout::SetWidth(int width) noexcept {
  if (width <= 0) {
    GetConsole().call<void>("warn", "Invalid Width value: " + std::to_string(width));
    return;
  }
  mWidth = width;
}

/// @brief Sets the height of the layout container; must be > 0.
/// @param height Height in pixels.
void WebLayout::SetHeight(int height) noexcept {
  if (height <= 0) {
    GetConsole().call<void>("warn", "Invalid Height value: " + std::to_string(height));
    return;
  }
  mHeight = height;
}

/// @brief Sets the CSS opacity of the layout container.
/// @param opacity Value in [0.0, 1.0]; 0.0 = transparent, 1.0 = opaque.
void WebLayout::SetOpacity(double opacity) noexcept {
  if (opacity < 0.0 || opacity > 1.0) {
    GetConsole().call<void>("warn", "Invalid Opacity value: " + std::to_string(opacity));
    return;
  }
  mOpacity = opacity;
}

/// @brief Sets the CSS box-shadow property of the layout container.
/// @param shadow CSS box-shadow string (e.g., "0 4px 6px rgba(0,0,0,0.1)").
void WebLayout::SetBoxShadow(const std::string& shadow) noexcept {
  mBoxShadow = shadow;
}

/// @brief Toggles this layout's visibility between visible and hidden.
void WebLayout::ToggleVisibility() noexcept { mIsVisible = !mIsVisible; }

// ===== IDomElement Interface Implementations =====

/// @brief Registers this layout as a child of @p parent and ensures the DOM
///        parent–child relationship is established.
/// @param parent Parent WebLayout to mount into.
/// @param align  Alignment of this layout within the parent.
void WebLayout::MountToLayout(WebLayout& parent, Alignment align) noexcept {
  parent.AddElement(this, align);
}

/// @brief Synchronizes the layout state with the DOM by calling Apply().
void WebLayout::SyncFromModel() noexcept {
  // default sync behavior: re-apply layout to reflect params/state
  Apply();
}

void WebLayout::RemoveChild(IDomElement * element) {
  // remove element from mChildren vector then from DOM
  RemoveElement(element);
}

void WebLayout::ApplyStyling(val style) noexcept {
  style.set("backgroundColor", mBackgroundColor);
  style.set("borderWidth", px(mBorderWidth));
  style.set("borderStyle", std::string("solid"));
  style.set("borderColor", mBorderColor);
  style.set("borderRadius", px(mBorderRadius));
  style.set("padding", px(mPadding));
  style.set("margin", px(mMargin));
  style.set("opacity", mOpacity);
  style.set("boxShadow", mBoxShadow);
  if (mWidth.has_value()) {
    style.set("width", px(mWidth.value()));
  }
  if (mHeight.has_value()) {
    style.set("height", px(mHeight.value()));
  }
}

void WebLayout::ApplyLayout(val style) noexcept {
  // Helper lambda to convert Justification to CSS justify-content string.
  auto getJustifyStr = [](Justification j) -> std::string {
    switch (j) {
      case Justification::Start:
        return "flex-start";
      case Justification::Center:
        return "center";
      case Justification::End:
        return "flex-end";
      case Justification::SpaceBetween:
        return "space-between";
      case Justification::SpaceAround:
        return "space-around";
      case Justification::SpaceEvenly:
        return "space-evenly";
    }
    assert(false && "Unhandled Justification");
  };

  // Helper lambda to convert Alignment to CSS align-items / justify-items
  // string.
  auto getAlignItemsStr = [](Alignment a) -> std::string {
    switch (a) {
      case Alignment::Start:
        return "flex-start";
      case Alignment::Center:
        return "center";
      case Alignment::End:
        return "flex-end";
      case Alignment::Stretch:
        return "stretch";
      case Alignment::None:
        return "none";
    }
    assert(false && "Unhandled Alignment");
  };

  switch (mType) {
    case LayoutType::Free:
      style.set("position", std::string("relative"));
      style.set("display",
                mIsVisible ? std::string("block") : std::string("none"));
      break;
    case LayoutType::Horizontal:
      style.set("display",
                mIsVisible ? std::string("flex") : std::string("none"));
      style.set("flexDirection", std::string("row"));
      style.set("alignItems", getAlignItemsStr(mAlignItems));
      style.set("justifyContent", getJustifyStr(mJustification));
      style.set("gap", px(mSpacing));
      break;
    case LayoutType::Vertical:
      style.set("display",
                mIsVisible ? std::string("flex") : std::string("none"));
      style.set("flexDirection", std::string("column"));
      style.set("alignItems", getAlignItemsStr(mAlignItems));
      style.set("justifyContent", getJustifyStr(mJustification));
      style.set("gap", px(mSpacing));
      break;
    case LayoutType::Grid:
      style.set("display",
                mIsVisible ? std::string("grid") : std::string("none"));
      style.set("alignItems", getAlignItemsStr(mAlignItems));
      style.set("justifyItems", getJustifyStr(mJustification));
      style.set("gap", px(mSpacing));
      break;
  }
}

void WebLayout::ApplyChildren() noexcept {
  for (IDomElement* elem : mChildren) {
    if (!elem) continue;
    const ElementID& id = elem->Id();
    if (id.empty()) continue;

    elem->SyncFromModel();

    val el = elem->GetElement();
    if (el.isNull() || el.isUndefined()) {
      GetConsole().call<void>("warn", "element with Id: '" + id + "' not found.");
      continue;
    }
    val est = el["style"];

    // apply alignment only; sizing is handled by element itself
    auto it = mParams.find(elem);
    if (it != mParams.end()) {
      Alignment a = it->second;
      std::string alignStr;
      switch (a) {
        case Alignment::Start:
          alignStr = "flex-start";
          break;
        case Alignment::Center:
          alignStr = "center";
          break;
        case Alignment::End:
          alignStr = "flex-end";
          break;
        case Alignment::Stretch:
          alignStr = "stretch";
          break;
        default:
          alignStr = "none";
      }
      est.set("alignSelf", alignStr);
    }

    // If grid layout and the element has grid coordinates, apply them
    if (mType == LayoutType::Grid) {
      int r = elem->GridRow();
      int c = elem->GridCol();
      if (r >= 0) {
        est.set("gridRowStart", std::to_string(r + 1));
      }
      if (c >= 0) {
        est.set("gridColumnStart", std::to_string(c + 1));
      }
    }

    // For free layout, apply positioning relative to mElement if set
    if (mType == LayoutType::Free) {
      int top = elem->FreeTop();
      int left = elem->FreeLeft();
      if (top >= 0 || left >= 0) {
        est.set("position", std::string("relative"));
        if (top >= 0) {
          est.set("top", px(top));
        }
        if (left >= 0) {
          est.set("left", px(left));
        }
      }
    }
  }
}

/// @brief Applies all pending layout and style changes to the DOM.
/// Calls ApplyStyling(), ApplyLayout(), and ApplyChildren() in order.
/// No-op when the layout is currently hidden (mIsVisible == false).
void WebLayout::Apply() noexcept {
  if (!mIsVisible) return;
  val style = mElement["style"];
  ApplyStyling(style);
  ApplyLayout(style);
  ApplyChildren();
}

/// @brief Unmounts and removes all child elements from this layout.
void WebLayout::Clear() noexcept {
  // remove children from DOM by unmounting them
  for (const auto& elem : mChildren) {
    elem->Unmount();
  }
  mChildren.clear();
  mParams.clear();
}

}  // namespace cse498

#endif
