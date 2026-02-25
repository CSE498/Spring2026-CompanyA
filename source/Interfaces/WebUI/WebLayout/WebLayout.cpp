#include "WebLayout.hpp"

#include <iostream>
#include <sstream>

using emscripten::val;

int WebLayout::mNextIdCounter = 1;

/// Helper function to convert a pixel value to a CSS string.
/// @param v Pixel value (non-negative)
/// @return CSS string like "10px" or empty string if v < 0
static std::string px(int v) noexcept {
  if (v < 0) return std::string();
  std::ostringstream ss;
  ss << v << "px";
  return ss.str();
}

/// Constructor initializes the WebLayout with a root DOM element.
/// If rootId is non-empty and an element with that ID exists in the DOM,
/// it is used as the root. Otherwise, a new div is created and appended to
/// body.
WebLayout::WebLayout(const std::string& rootId) noexcept {
  mDocument = val::global("document");
  if (!rootId.empty()) {
    val found = mDocument.call<val>("getElementById", rootId);
    if (!found.isNull() && !found.isUndefined()) {
      mRoot = found;
    } else {
      // create a container div with the requested id and append to body
      mRoot = mDocument.call<val>("createElement", std::string("div"));
      mRoot.set("id", rootId);
      mDocument["body"].call<void>("appendChild", mRoot);
    }
    mId = rootId;
    val style = mRoot["style"];
    style.set("boxSizing", std::string("border-box"));
    return;
  }

  // no id passed: create a container div and append to body with generated id
  mId = "weblayout-" + std::to_string(mNextIdCounter++);
  mRoot = mDocument.call<val>("createElement", std::string("div"));
  mRoot.set("id", mId);
  val style = mRoot["style"];
  style.set("boxSizing", std::string("border-box"));
  mDocument["body"].call<void>("appendChild", mRoot);
  assert(!mRoot.isNull() && !mRoot.isUndefined());
  assert(mRoot["id"].as<std::string>() == mId);
}

/// Destructor removes the root element from the DOM and cleans up references.
WebLayout::~WebLayout() noexcept {
  // remove the root element from DOM if present
  val parent = mRoot["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", mRoot);
  }
  mDocument = val::undefined();
  mRoot = val::undefined();
}

/// Sets the layout type and stores it for later application to the DOM.
void WebLayout::SetLayoutType(LayoutType t) noexcept { mType = t; }

/// Sets the justification mode for the main axis alignment.
void WebLayout::SetJustification(Justification j) noexcept {
  mJustification = j;
}

/// Sets the cross-axis alignment for child elements.
void WebLayout::SetAlignItems(Alignment a) noexcept { mAlignItems = a; }

/// Sets the gap/spacing between child elements.
void WebLayout::SetSpacing(int pxv) noexcept {
  assert(pxv >= 0);
  mSpacing = pxv;
}

/// Adds a child element to this layout.
/// The element is appended to the root's DOM node and tracked for later Apply()
/// calls.
bool WebLayout::AddElement(IDomElement* elem, Alignment align) noexcept {
  if (!elem) return false;
  const ElementID& id = elem->Id();
  if (id.empty()) return false;

  val el = mDocument.call<val>("getElementById", id);
  if (el.isNull() || el.isUndefined()) {
    return false;
  }

  // attach now; Apply() will re-check parent when re-applying
  mRoot.call<void>("appendChild", el);
  mChildren.push_back(elem);
  mParams[elem] = align;
  return true;
}

/// Removes a child element from this layout and removes it from the DOM.
bool WebLayout::RemoveElement(IDomElement* elem) noexcept {
  if (!elem) return false;
  auto it = std::find(mChildren.begin(), mChildren.end(), elem);
  if (it == mChildren.end()) return false;
  mChildren.erase(it);
  mParams.erase(elem);

  const ElementID& id = elem->Id();
  val el = mDocument.call<val>("getElementById", id);
  if (!el.isNull() && !el.isUndefined()) {
    val parent = el["parentNode"];
    if (!parent.isNull() && !parent.isUndefined()) {
      parent.call<void>("removeChild", el);
    }
  }
  return true;
}

/// Updates the alignment of a specific child element.
void WebLayout::SetAlignment(IDomElement* elem, Alignment a) noexcept {
  if (!elem) return;
  auto it = mParams.find(elem);
  if (it == mParams.end()) return;
  it->second = a;
}

// ===== Styling Methods =====

/// Sets the background color for this layout.
void WebLayout::SetBackgroundColor(const std::string& color) noexcept {
  mBackgroundColor = color;
}

/// Sets the border color for this layout.
void WebLayout::SetBorderColor(const std::string& color) noexcept {
  mBorderColor = color;
}

/// Sets the border width in pixels.
void WebLayout::SetBorderWidth(int width) noexcept {
  assert(width >= 0);
  mBorderWidth = width;
}

/// Sets the border radius for rounded corners.
void WebLayout::SetBorderRadius(int radius) noexcept {
  assert(radius >= 0);
  mBorderRadius = radius;
}

/// Sets the padding in pixels.
void WebLayout::SetPadding(int padding) noexcept {
  assert(padding >= 0);
  mPadding = padding;
}

/// Sets the margin in pixels.
void WebLayout::SetMargin(int margin) noexcept {
  assert(margin >= 0);
  mMargin = margin;
}

/// Sets the width in pixels.
void WebLayout::SetWidth(int width) noexcept {
  assert(width > 0);
  mWidth = width;
}

/// Sets the height in pixels.
void WebLayout::SetHeight(int height) noexcept {
  assert(height > 0);
  mHeight = height;
}

/// Sets the opacity/transparency level (0.0 = transparent, 1.0 = opaque).
void WebLayout::SetOpacity(double opacity) noexcept {
  assert(opacity >= 0.0 && opacity <= 1.0);
  mOpacity = opacity;
}

/// Sets the box shadow CSS property.
void WebLayout::SetBoxShadow(const std::string& shadow) noexcept {
  mBoxShadow = shadow;
}

/// Toggles the visibility of this layout between hidden and visible.
void WebLayout::ToggleVisibility() noexcept { mIsVisible = !mIsVisible; }

// ===== IDomElement Interface Implementations =====
/// Mounts this layout as a child within a parent layout.
/// Registers with the parent and establishes the DOM parent-child relationship.
void WebLayout::MountToLayout(WebLayout& parent, Alignment align) noexcept {
  // register this layout element with parent and ensure DOM parent-child
  // relationship
  parent.AddElement(this, align);
}

/// Unmounts this layout from its DOM parent.
void WebLayout::Unmount() noexcept {
  val parent = mRoot["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", mRoot);
  }
}

/// Synchronizes the layout state with the DOM by calling Apply().
void WebLayout::SyncFromModel() noexcept {
  // default sync behavior: re-apply layout to reflect params/state
  Apply();
}

/// Applies all layout configuration and styling properties to the DOM.
/// This method synchronizes the stored state with the actual DOM element
/// and all child elements. Call this after making configuration changes.
void WebLayout::Apply() noexcept {
  val style = mRoot["style"];

  // Apply styling properties
  if (!mBackgroundColor.empty()) {
    style.set("backgroundColor", mBackgroundColor);
  }
  if (mBorderWidth > 0) {
    style.set("borderWidth", px(mBorderWidth));
    style.set("borderStyle", std::string("solid"));
    if (!mBorderColor.empty()) {
      style.set("borderColor", mBorderColor);
    }
  }
  if (mBorderRadius > 0) {
    style.set("borderRadius", px(mBorderRadius));
  }
  if (mPadding > 0) {
    style.set("padding", px(mPadding));
  }
  if (mMargin > 0) {
    style.set("margin", px(mMargin));
  }
  if (mWidth > 0) {
    style.set("width", px(mWidth));
  }
  if (mHeight > 0) {
    style.set("height", px(mHeight));
  }
  if (mOpacity < 1.0) {
    style.set("opacity", mOpacity);
  }
  if (!mBoxShadow.empty()) {
    style.set("boxShadow", mBoxShadow);
  }

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
      default:
        return "flex-start";
    }
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
      default:
        return "none";
    }
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

  // ensure ordering and apply per-child styles
  for (IDomElement* elem : mChildren) {
    if (!elem) continue;
    const ElementID& id = elem->Id();
    if (id.empty()) continue;

    val el = mDocument.call<val>("getElementById", id);
    if (el.isNull() || el.isUndefined()) continue;

    // apply alignment only; sizing is handled by element itself
    auto it = mParams.find(elem);
    if (it != mParams.end()) {
      Alignment a = it->second;
      val est = el["style"];
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
      val est = el["style"];
      if (r >= 0) {
        est.set("gridRowStart", std::to_string(r + 1));
      }
      if (c >= 0) {
        est.set("gridColumnStart", std::to_string(c + 1));
      }
    }

    // For free layout, apply positioning relative to mRoot if set
    if (mType == LayoutType::Free) {
      int top = elem->FreeTop();
      int left = elem->FreeLeft();
      val est = el["style"];
      if (top >= 0 || left >= 0) {
        est.set("position", std::string("relative"));
        if (top >= 0) {
          est.set("position", std::string("relative"));
          est.set("top", px(top));
        }
        if (left >= 0) {
          est.set("left", px(left));
        }
      }
    }
  }
}

/// Clears all child elements from the layout and removes them from the DOM.
void WebLayout::Clear() noexcept {
  // remove children from DOM by clearing innerHTML
  mRoot.set("innerHTML", std::string(""));
  mChildren.clear();
  mParams.clear();
}
