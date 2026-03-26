#ifdef __EMSCRIPTEN__

#include "WebLayout.hpp"

#include <string>

using emscripten::val;

namespace cse498 {

int WebLayout::mNextIdCounter = 1;

/// Helper function to convert a pixel value to a CSS string.
/// @param v Pixel value (non-negative)
/// @return CSS string like "10px" or empty string if v < 0
static std::string px(int v) noexcept {
  if (v < 0)
    return std::string();
  return std::to_string(v) + "px";
}

WebLayout::WebLayout(const std::string &rootId) noexcept {
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

WebLayout::~WebLayout() noexcept {
  // unmount children
  Clear();

  // remove the root element from DOM if present
  val parent = mRoot["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", mRoot);
  }
  mDocument = val::undefined();
  mRoot = val::undefined();
}

void WebLayout::SetLayoutType(LayoutType t) noexcept { mType = t; }

void WebLayout::SetJustification(Justification j) noexcept {
  mJustification = j;
}

void WebLayout::SetAlignItems(Alignment a) noexcept { mAlignItems = a; }

/// Sets the gap/spacing between child elements.
void WebLayout::SetSpacing(int pxv) noexcept {
  assert(pxv >= 0);
  mSpacing = pxv;
}

bool WebLayout::AddElement(IDomElement *elem, Alignment align) noexcept {
  if (!elem)
    return false;
  const ElementID &id = elem->Id();
  if (id.empty())
    return false;

  val el = mDocument.call<val>("getElementById", id);
  if (el.isNull() || el.isUndefined()) {
    return false;
  }

  // attach now; Apply() will re-check parent when re-applying
  // if element already exists in this layout, this will just move it to the end
  mRoot.call<void>("appendChild", el);

  // if element already exists in this layout, don't add it to mChildren again
  if (std::find(mChildren.begin(), mChildren.end(), elem) != mChildren.end())
    return true;

  mChildren.push_back(elem);
  mParams[elem] = align;
  return true;
}

bool WebLayout::RemoveElement(IDomElement *elem) noexcept {
  if (!elem)
    return false;
  auto it = std::find(mChildren.begin(), mChildren.end(), elem);
  if (it == mChildren.end())
    return false;
  (*it)->Unmount();
  mChildren.erase(it);
  mParams.erase(elem);

  const ElementID &id = elem->Id();
  val el = mDocument.call<val>("getElementById", id);
  if (!el.isNull() && !el.isUndefined()) {
    if (mRoot == el["parentNode"]) {
      mRoot.call<void>("removeChild", el);
    }
  }
  return true;
}

void WebLayout::SetAlignment(IDomElement *elem, Alignment a) noexcept {
  if (!elem)
    return;
  auto it = mParams.find(elem);
  if (it == mParams.end())
    return;
  it->second = a;
}

// ===== Styling Methods =====

void WebLayout::SetBackgroundColor(const std::string &color) noexcept {
  mBackgroundColor = color;
}

void WebLayout::SetBorderColor(const std::string &color) noexcept {
  mBorderColor = color;
}

void WebLayout::SetBorderWidth(int width) noexcept {
  assert(width >= 0);
  mBorderWidth = width;
}

void WebLayout::SetBorderRadius(int radius) noexcept {
  assert(radius >= 0);
  mBorderRadius = radius;
}

void WebLayout::SetPadding(int padding) noexcept {
  assert(padding >= 0);
  mPadding = padding;
}

void WebLayout::SetMargin(int margin) noexcept {
  assert(margin >= 0);
  mMargin = margin;
}

void WebLayout::SetWidth(int width) noexcept {
  assert(width > 0);
  mWidth = width;
}

void WebLayout::SetHeight(int height) noexcept {
  assert(height > 0);
  mHeight = height;
}

void WebLayout::SetOpacity(double opacity) noexcept {
  assert(opacity >= 0.0 && opacity <= 1.0);
  mOpacity = opacity;
}

void WebLayout::SetBoxShadow(const std::string &shadow) noexcept {
  mBoxShadow = shadow;
}

void WebLayout::ToggleVisibility() noexcept { mIsVisible = !mIsVisible; }

// ===== IDomElement Interface Implementations =====

void WebLayout::MountToLayout(WebLayout &parent, Alignment align) noexcept {
  // register this layout element with parent and ensure DOM parent-child
  // relationship
  parent.AddElement(this, align);
}

void WebLayout::Unmount() noexcept {
  // destroying nested layouts can cause mRoot to be undefined
  if (mRoot.isNull() || mRoot.isUndefined())
    return;
  val parent = mRoot["parentNode"];
  if (!parent.isNull() && !parent.isUndefined()) {
    parent.call<void>("removeChild", mRoot);
  }
}

void WebLayout::SyncFromModel() noexcept {
  // default sync behavior: re-apply layout to reflect params/state
  Apply();
}

void WebLayout::ApplyStyling(val &style) noexcept {
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
}

void WebLayout::ApplyLayout(val &style) noexcept {
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
  for (IDomElement *elem : mChildren) {
    if (!elem)
      continue;
    const ElementID &id = elem->Id();
    if (id.empty())
      continue;

    elem->SyncFromModel();

    val el = mDocument.call<val>("getElementById", id);
    if (el.isNull() || el.isUndefined())
      continue;
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

    // For free layout, apply positioning relative to mRoot if set
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

void WebLayout::Apply() noexcept {
  if (!mIsVisible)
    return;
  val style = mRoot["style"];
  ApplyStyling(style);
  ApplyLayout(style);
  ApplyChildren();
}

void WebLayout::Clear() noexcept {
  // remove children from DOM by unmounting them
  for (const auto &elem : mChildren) {
    elem->Unmount();
  }
  mChildren.clear();
  mParams.clear();
}

} // namespace cse498

#endif
