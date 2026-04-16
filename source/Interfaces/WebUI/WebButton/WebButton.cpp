/**
 * @file WebButton.cpp
 * @brief Implementation of WebButton, a DOM-backed clickable button for WebUI.
 *
 * Contains the DOM interaction logic for creating a \<button\> element,
 * attaching a JavaScript click listener that forwards events back to C++,
 * and keeping the element's state synchronized with the model.
 *
 */

#include "WebButton.hpp"
#include <cassert>
#include <emscripten.h>
#include "../../../tools/Color.hpp"
#include "../WebCanvas/WebCanvas.hpp"
#include "../WebLayout/WebLayout.hpp"

using emscripten::val;

namespace cse498 {

int WebButton::mNextIdCounter = 1;

namespace {
constexpr std::string_view kDefaultBgColorHex = "#4a90d9";
constexpr std::string_view kDisabledBgColorHex = "#aaaaaa";
constexpr std::string_view kDefaultTextColorHex = "#ffffff";
static_assert(cse498::Color::FromHex(kDefaultBgColorHex).has_value());
static_assert(cse498::Color::FromHex(kDisabledBgColorHex).has_value());
static_assert(cse498::Color::FromHex(kDefaultTextColorHex).has_value());
} // namespace

/// @brief Converts an integer pixel value to a CSS pixel string (e.g., "42px").
/// @param value Pixel value to convert.
/// @return String of the form "<value>px".
static std::string ToPx(int value) { return std::to_string(value) + "px"; }

/// @brief Constructs a WebButton, creates a \<button\> DOM element, and
///        attaches the JS click listener.
/// @param label Initial button label text.
WebButton::WebButton(const std::string& label) : mLabel(label) {
    mId = std::string(kIdPrefix) + std::to_string(mNextIdCounter++);

    val doc = GetDocument();
    mElement = doc.call<val>("createElement", std::string("button"));
    mElement.set("id", mId);
    mElement.set("textContent", mLabel);
    mElement["style"].set("boxSizing", std::string("border-box"));

    AttachClickListener();
}

/// @brief Destructor: unmounts the button from the DOM and releases the element.
WebButton::~WebButton() {
    Unmount();
    mElement = val::null();
}

/// @brief Move constructor: transfers DOM ownership and state from @p other.
/// @param other Source WebButton to move from.
WebButton::WebButton(WebButton&& other) noexcept :
    mLabel(std::move(other.mLabel)), mCallback(std::move(other.mCallback)), mIsEnabled(other.mIsEnabled),
    mIsVisible(other.mIsVisible), mWidth(other.mWidth), mHeight(other.mHeight), mBgColor(std::move(other.mBgColor)),
    mTextColor(std::move(other.mTextColor)) {
    mId = std::move(other.mId);
    mElement = std::move(other.mElement);
    other.mElement = val::null();
    other.mIsEnabled = false;
    other.mIsVisible = false;
    other.mWidth = kDefaultSize;
    other.mHeight = kDefaultSize;
}

/// @brief Move assignment: unmounts current element then transfers state from @p other.
/// @param other Source WebButton to move from.
/// @return Reference to this object.
WebButton& WebButton::operator=(WebButton&& other) noexcept {
    if (this != &other) {
        Unmount();

        mLabel = std::move(other.mLabel);
        mCallback = std::move(other.mCallback);
        mIsEnabled = other.mIsEnabled;
        mIsVisible = other.mIsVisible;
        mWidth = other.mWidth;
        mHeight = other.mHeight;
        mBgColor = std::move(other.mBgColor);
        mTextColor = std::move(other.mTextColor);
        mElement = other.mElement;
        mId = std::move(other.mId);

        other.mElement = val::null();
        other.mIsEnabled = false;
        other.mIsVisible = false;
        other.mWidth = kDefaultSize;
        other.mHeight = kDefaultSize;
    }
    return *this;
}

/// @brief Sets the button label and updates the DOM textContent.
/// @param text New label string.
void WebButton::SetLabel(const std::string& text) {
    mLabel = text;
    if (!mElement.isNull()) {
        mElement.set("textContent", mLabel);
    }
}

/// @brief Returns the current button label.
/// @return Copy of the current label string.
std::string WebButton::GetLabel() const { return mLabel; }

/// @brief Registers the click callback; asserts that the callback is non-null.
/// @param callback Callable invoked on button click.
void WebButton::SetCallback(std::function<void()> callback) {
    assert(callback && "SetCallback: callback must not be null");
    mCallback = std::move(callback);
}

/// @brief Fires the click callback if the button is enabled and a callback is set.
void WebButton::Click() {
    if (mIsEnabled && mCallback) {
        mCallback();
    }
}

/// @brief Sets button dimensions; 0 means use the browser default.
/// @param width  Width in pixels (must be >= 0).
/// @param height Height in pixels (must be >= 0).
void WebButton::SetSize(int width, int height) {
    assert(width >= 0 && "SetSize: width must be non-negative");
    assert(height >= 0 && "SetSize: height must be non-negative");
    mWidth = width;
    mHeight = height;
    ApplyStyle("width", width > 0 ? ToPx(width) : "auto");
    ApplyStyle("height", height > 0 ? ToPx(height) : "auto");
}

/// @brief Returns the current width in pixels (0 = browser default).
/// @return Width in pixels.
int WebButton::GetWidth() const { return mWidth; }

/// @brief Returns the current height in pixels (0 = browser default).
/// @return Height in pixels.
int WebButton::GetHeight() const { return mHeight; }

/// @brief Sets the button background color and applies it to the DOM element.
/// @param color CSS color string.
void WebButton::SetBackgroundColor(const std::string& color) {
    mBgColor = color;
    ApplyStyle("backgroundColor", color);
}

/// @brief Sets the button text color and applies it to the DOM element.
/// @param color CSS color string.
void WebButton::SetTextColor(const std::string& color) {
    mTextColor = color;
    ApplyStyle("color", color);
}

/// @brief Enables the button and removes the disabled attribute from the DOM element.
void WebButton::Enable() {
    mIsEnabled = true;
    if (!mElement.isNull()) {
        mElement.set("disabled", false);
    }
}

/// @brief Disables the button and sets the disabled attribute on the DOM element.
void WebButton::Disable() {
    mIsEnabled = false;
    if (!mElement.isNull()) {
        mElement.set("disabled", true);
    }
}

/// @brief Returns whether the button is currently enabled.
/// @return True if enabled; false if disabled.
bool WebButton::IsEnabled() const { return mIsEnabled; }

/// @brief Makes the button visible by clearing the CSS display property.
void WebButton::Show() {
    mIsVisible = true;
    ApplyStyle("display", "");
}

/// @brief Hides the button by setting display: none on the DOM element.
void WebButton::Hide() {
    mIsVisible = false;
    ApplyStyle("display", "none");
}

/// @brief Returns whether the button is currently visible.
/// @return True if visible; false if hidden.
bool WebButton::IsVisible() const { return mIsVisible; }

/// @brief Mounts this button into the given parent layout.
/// @param parent Parent WebLayout to attach to.
/// @param align  Alignment within the parent container.
void WebButton::MountToLayout(WebLayout& parent, Alignment align) { parent.AddElement(this, align); }

/// @brief Synchronizes the DOM element with the current model state.
void WebButton::SyncFromModel() {
    if (mElement.isNull())
        return;

    mElement.set("textContent", mLabel);
    mElement.set("disabled", !mIsEnabled);

    ApplyStyle("width", mWidth > 0 ? ToPx(mWidth) : "auto");
    ApplyStyle("height", mHeight > 0 ? ToPx(mHeight) : "auto");

    if (!mBgColor.empty()) {
        ApplyStyle("backgroundColor", mBgColor);
    }
    if (!mTextColor.empty()) {
        ApplyStyle("color", mTextColor);
    }

    ApplyStyle("display", mIsVisible ? "" : "none");
}

/// @brief Forwards the JS click event to Click(); called by the C trampoline.
void WebButton::HandleClick() { Click(); }

/// @brief Attaches a JS click event listener that forwards events to HandleClick()
///        via the WebButton_handleClick C trampoline.
void WebButton::AttachClickListener() {
    if (mElement.isNull())
        return;

    EM_ASM(
            {
                var el = Emval.toValue($0);
                var ptrVal = $1;
                el.addEventListener("click", function() { Module._WebButton_handleClick(ptrVal); });
            },
            mElement.as_handle(), reinterpret_cast<intptr_t>(this));
}

/// @brief Applies a CSS style property to the DOM element if it exists.
/// @param property CSS property name.
/// @param value CSS property value.
void WebButton::ApplyStyle(const std::string& property, const std::string& value) {
    if (!mElement.isNull()) {
        mElement["style"].set(property, value);
    }
}

// ----- ICanvasElement Interface -----

/// @brief Stores the canvas-space rectangle used by Draw().
void WebButton::SetCanvasRect(float x, float y, float w, float h) {
    mCanvasX = x;
    mCanvasY = y;
    mCanvasW = w;
    mCanvasH = h;
}

/// @brief Draws the button background rectangle and label text onto @p canvas.
/// Disabled buttons use a grey background; enabled buttons use mBgColor (or a
/// default blue if none is set). The label is drawn centered in the rect.
void WebButton::Draw(WebCanvas& canvas) {
    float w = (mCanvasW > 0) ? mCanvasW : static_cast<float>(mWidth > 0 ? mWidth : kDefaultCanvasWidth);
    float h = (mCanvasH > 0) ? mCanvasH : static_cast<float>(mHeight > 0 ? mHeight : kDefaultCanvasHeight);

    std::string bg = mIsEnabled ? (mBgColor.empty() ? std::string(kDefaultBgColorHex) : mBgColor)
                                : std::string(kDisabledBgColorHex);

    canvas.DrawRect(mCanvasX, mCanvasY, w, h, bg);

    if (!mLabel.empty()) {
        std::string textCol = mTextColor.empty() ? std::string(kDefaultTextColorHex) : mTextColor;
        float tx = mCanvasX + w * 0.5f - static_cast<float>(mLabel.size()) * kDefaultFontSize * kLabelHorizontalFactor;
        float ty = mCanvasY + h * kLabelVerticalCenter;
        canvas.DrawText(tx, ty, mLabel, textCol, kDefaultFontSize, "");
    }
}

} // namespace cse498

/// @brief C trampoline called from the JS click listener; looks up the WebButton
///        by pointer and calls HandleClick().
/// @param ptr Opaque integer encoding a WebButton* pointer.
extern "C" {
EMSCRIPTEN_KEEPALIVE
void WebButton_handleClick(intptr_t ptr) {
    auto* btn = reinterpret_cast<cse498::WebButton*>(ptr);
    if (btn) {
        btn->HandleClick();
    }
}
}
