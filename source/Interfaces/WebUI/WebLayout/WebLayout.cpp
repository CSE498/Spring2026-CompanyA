#include "WebLayout.hpp"

#include <iostream>
#include <sstream>

using emscripten::val;

int WebLayout::nextIdCounter_ = 1;

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
  document_ = val::global("document");
  if (!rootId.empty()) {
    val found = document_.call<val>("getElementById", rootId);
    if (!found.isNull() && !found.isUndefined()) {
      root_ = found;
    } else {
      // create a container div with the requested id and append to body
      root_ = document_.call<val>("createElement", std::string("div"));
      root_.set("id", rootId);
      document_["body"].call<void>("appendChild", root_);
    }
    id_ = rootId;
    val style = root_["style"];
    style.set("boxSizing", std::string("border-box"));
    return;
  }

  // no id passed: create a container div and append to body with generated id
  id_ = "weblayout-" + std::to_string(nextIdCounter_++);
  root_ = document_.call<val>("createElement", std::string("div"));
  root_.set("id", id_);
  val style = root_["style"];
  style.set("boxSizing", std::string("border-box"));
  document_["body"].call<void>("appendChild", root_);
  assert(!root_.isNull() && !root_.isUndefined());
  assert(root_["id"].as<std::string>() == id_);
}

/// Destructor removes the root element from the DOM and cleans up references.
WebLayout::~WebLayout() noexcept {
  // remove the root element from DOM if present
  val parent = root_["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", root_);
  }
  document_ = val::undefined();
  root_ = val::undefined();
}

/// Sets the layout type and stores it for later application to the DOM.
void WebLayout::SetLayoutType(LayoutType t) noexcept { type_ = t; }

/// Sets the justification mode for the main axis alignment.
void WebLayout::SetJustification(Justification j) noexcept {
  justification_ = j;
}

/// Sets the cross-axis alignment for child elements.
void WebLayout::SetAlignItems(Alignment a) noexcept { alignItems_ = a; }

/// Sets the gap/spacing between child elements.
void WebLayout::SetSpacing(int pxv) noexcept {
  assert(pxv >= 0);
  spacing_ = pxv;
}

/// Adds a child element to this layout.
/// The element is appended to the root's DOM node and tracked for later Apply()
/// calls.
bool WebLayout::AddElement(IDomElement* elem, Alignment align) noexcept {
  if (!elem) return false;
  const ElementID& id = elem->Id();
  if (id.empty()) return false;

  val el = document_.call<val>("getElementById", id);
  if (el.isNull() || el.isUndefined()) {
    return false;
  }

  // attach now; Apply() will re-check parent when re-applying
  root_.call<void>("appendChild", el);
  children_.push_back(elem);
  params_[elem] = align;
  return true;
}

/// Removes a child element from this layout and removes it from the DOM.
bool WebLayout::RemoveElement(IDomElement* elem) noexcept {
  if (!elem) return false;
  auto it = std::find(children_.begin(), children_.end(), elem);
  if (it == children_.end()) return false;
  children_.erase(it);
  params_.erase(elem);

  const ElementID& id = elem->Id();
  val el = document_.call<val>("getElementById", id);
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
  auto it = params_.find(elem);
  if (it == params_.end()) return;
  it->second = a;
}

// ===== Styling Methods =====

/// Sets the background color for this layout.
void WebLayout::SetBackgroundColor(const std::string& color) noexcept {
  backgroundColor_ = color;
}

/// Sets the border color for this layout.
void WebLayout::SetBorderColor(const std::string& color) noexcept {
  borderColor_ = color;
}

/// Sets the border width in pixels.
void WebLayout::SetBorderWidth(int width) noexcept {
  assert(width >= 0);
  borderWidth_ = width;
}

/// Sets the border radius for rounded corners.
void WebLayout::SetBorderRadius(int radius) noexcept {
  assert(radius >= 0);
  borderRadius_ = radius;
}

/// Sets the padding in pixels.
void WebLayout::SetPadding(int padding) noexcept {
  assert(padding >= 0);
  padding_ = padding;
}

/// Sets the margin in pixels.
void WebLayout::SetMargin(int margin) noexcept {
  assert(margin >= 0);
  margin_ = margin;
}

/// Sets the width in pixels.
void WebLayout::SetWidth(int width) noexcept {
  assert(width > 0);
  width_ = width;
}

/// Sets the height in pixels.
void WebLayout::SetHeight(int height) noexcept {
  assert(height > 0);
  height_ = height;
}

/// Sets the opacity/transparency level (0.0 = transparent, 1.0 = opaque).
void WebLayout::SetOpacity(double opacity) noexcept {
  assert(opacity >= 0.0 && opacity <= 1.0);
  opacity_ = opacity;
}

/// Sets the box shadow CSS property.
void WebLayout::SetBoxShadow(const std::string& shadow) noexcept {
  boxShadow_ = shadow;
}

/// Toggles the visibility of this layout between hidden and visible.
void WebLayout::ToggleVisibility() noexcept { isVisible_ = !isVisible_; }

// ===== IDomElement Interface Implementations =====
/// Mounts this layout as a child within a parent layout.
/// Registers with the parent and establishes the DOM parent-child relationship.
void WebLayout::mountToLayout(WebLayout& parent, Alignment align) noexcept {
  // register this layout element with parent and ensure DOM parent-child
  // relationship
  parent.AddElement(this, align);
}

/// Unmounts this layout from its DOM parent.
void WebLayout::unmount() noexcept {
  val parent = root_["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", root_);
  }
}

/// Synchronizes the layout state with the DOM by calling Apply().
void WebLayout::syncFromModel() noexcept {
  // default sync behavior: re-apply layout to reflect params/state
  Apply();
}

/// Applies all layout configuration and styling properties to the DOM.
/// This method synchronizes the stored state with the actual DOM element
/// and all child elements. Call this after making configuration changes.
void WebLayout::Apply() noexcept {
  val style = root_["style"];

  // Apply styling properties
  if (!backgroundColor_.empty()) {
    style.set("backgroundColor", backgroundColor_);
  }
  if (borderWidth_ > 0) {
    style.set("borderWidth", px(borderWidth_));
    style.set("borderStyle", std::string("solid"));
    if (!borderColor_.empty()) {
      style.set("borderColor", borderColor_);
    }
  }
  if (borderRadius_ > 0) {
    style.set("borderRadius", px(borderRadius_));
  }
  if (padding_ > 0) {
    style.set("padding", px(padding_));
  }
  if (margin_ > 0) {
    style.set("margin", px(margin_));
  }
  if (width_ > 0) {
    style.set("width", px(width_));
  }
  if (height_ > 0) {
    style.set("height", px(height_));
  }
  if (opacity_ < 1.0) {
    style.set("opacity", opacity_);
  }
  if (!boxShadow_.empty()) {
    style.set("boxShadow", boxShadow_);
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

  switch (type_) {
    case LayoutType::Free:
      style.set("position", std::string("relative"));
      style.set("display",
                isVisible_ ? std::string("block") : std::string("none"));
      break;
    case LayoutType::Horizontal:
      style.set("display",
                isVisible_ ? std::string("flex") : std::string("none"));
      style.set("flexDirection", std::string("row"));
      style.set("alignItems", getAlignItemsStr(alignItems_));
      style.set("justifyContent", getJustifyStr(justification_));
      style.set("gap", px(spacing_));
      break;
    case LayoutType::Vertical:
      style.set("display",
                isVisible_ ? std::string("flex") : std::string("none"));
      style.set("flexDirection", std::string("column"));
      style.set("alignItems", getAlignItemsStr(alignItems_));
      style.set("justifyContent", getJustifyStr(justification_));
      style.set("gap", px(spacing_));
      break;
    case LayoutType::Grid:
      style.set("display",
                isVisible_ ? std::string("grid") : std::string("none"));
      style.set("alignItems", getAlignItemsStr(alignItems_));
      style.set("justifyItems", getJustifyStr(justification_));
      style.set("gap", px(spacing_));
      break;
  }

  // ensure ordering and apply per-child styles
  for (IDomElement* elem : children_) {
    if (!elem) continue;
    const ElementID& id = elem->Id();
    if (id.empty()) continue;

    val el = document_.call<val>("getElementById", id);
    if (el.isNull() || el.isUndefined()) continue;

    // apply alignment only; sizing is handled by element itself
    auto it = params_.find(elem);
    if (it != params_.end()) {
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
    if (type_ == LayoutType::Grid) {
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

    // For free layout, apply positioning relative to root_ if set
    if (type_ == LayoutType::Free) {
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
  root_.set("innerHTML", std::string(""));
  children_.clear();
  params_.clear();
}
