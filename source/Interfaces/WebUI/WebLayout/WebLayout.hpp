/**
 * @file WebLayout.hpp
 * @brief Declaration of WebLayout, a DOM container supporting Flexbox, Grid,
 *        and Free positioning of IDomElement children.
 *
 * WebLayout creates and manages a single root <div> element in the browser
 * DOM and exposes a high-level C++ API for configuring its layout model,
 * styling, and child elements. It also implements IDomElement so that layouts
 * can be nested inside other layouts.
 *
 */

#pragma once

#include <emscripten/val.h>

#include <cassert>
#include <functional>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "../internal/IDomElement.hpp"

namespace cse498 {

    using ElementID = std::string;

    /// Enum defining the layout type for organizing child elements.
    /// - Free: Absolute/relative positioning layout
    /// - Horizontal: Flex row layout
    /// - Vertical: Flex column layout
    /// - Grid: CSS Grid layout
    enum class LayoutType { Free, Horizontal, Vertical, Grid, None };

    /// Enum defining justification (main axis alignment) for layout items.
    /// - Start: Align items to the start of the main axis
    /// - Center: Center items along the main axis
    /// - End: Align items to the end of the main axis
    /// - SpaceBetween: Distribute with equal space between items
    /// - SpaceAround: Distribute with equal space around items
    /// - SpaceEvenly: Distribute with equal space around and between items
    enum class Justification { Start, Center, End, SpaceBetween, SpaceAround, SpaceEvenly, None };

    /// WebLayout manages a DOM container element and arranges child IDomElements
    /// using various layout models (Flexbox, Grid, or Free positioning).
    ///
    /// This class:
    /// - Creates and manages a root DOM element
    /// - Provides layout configuration (type, justification, alignment)
    /// - Supports adding/removing child elements dynamically
    /// - Applies comprehensive styling (colors, borders, spacing, shadows)
    /// - Synchronizes layout state with the DOM via the Apply() method
    ///
    /// The WebLayout also implements the IDomElement interface, allowing it to be
    /// nested within other WebLayout instances.
    class WebLayout : public IDomElement {
    public:
        /// Constructs a WebLayout with an optional root element ID.
        /// If rootId is provided and exists in the DOM, that element is used as root.
        /// Otherwise, a new div is created and appended to the document body.
        /// @param rootId Optional ID of an existing DOM element or desired ID for new
        /// element
        explicit WebLayout(const std::string& rootId = "") noexcept;

        /// Deleted copy constructor and assignment operator to prevent copying.
        WebLayout(const WebLayout&) = delete;
        WebLayout& operator=(const WebLayout&) = delete;
        WebLayout(WebLayout&&) = delete;

        /// Destructor: Removes the root element from the DOM if present.
        ~WebLayout() noexcept;

        // ===== IDomElement interface =====

        /// Mounts this layout as a child within a parent layout.
        /// @param parent The parent layout to mount into
        /// @param align Alignment setting for this layout within the parent
        void MountToLayout(WebLayout& parent, Alignment align = Alignment::None) noexcept override;

        void RemoveChild(IDomElement* elem) override;

        /// Synchronizes the layout state with the DOM. Calls Apply() to refresh.
        void SyncFromModel() noexcept override;

        // ===== Layout Configuration =====

        /// Sets the layout type for organizing child elements.
        /// @param t The LayoutType (Free, Horizontal, Vertical, or Grid)
        void SetLayoutType(LayoutType t) noexcept;

        /// Sets the justification (main axis alignment) for child elements.
        /// @param j The Justification enum value
        void SetJustification(Justification j) noexcept;

        /// Sets the cross-axis alignment for child elements.
        /// @param a The Alignment for the cross axis
        void SetAlignItems(Alignment a) noexcept;

        // ===== Child Element Management =====

        /// Adds a child element to this layout and appends it to the DOM.
        /// @param elem Pointer to the child IDomElement
        /// @param align Optional alignment for this specific child
        /// @return true if successfully added, false if elem is null or invalid
        bool AddElement(IDomElement* elem, Alignment align = Alignment::None) noexcept;

        /// Removes a child element from this layout and the DOM.
        /// it cannot be added again until re-added to the DOM.
        /// @param elem Pointer to the child element to remove
        /// @return true if successfully removed, false if elem not found
        bool RemoveElement(IDomElement* elem) noexcept;

        /// Updates the alignment of a specific child element.
        /// @param elem Pointer to the child element
        /// @param a The new Alignment value for this child
        void SetAlignment(IDomElement* elem, Alignment a) noexcept;

        // ===== Styling Methods =====

        /// Sets the spacing (gap) between child elements in pixels.
        /// @param px The spacing in pixels (must be >= 0)
        void SetSpacing(int px) noexcept;

        /// Sets the background color.
        /// @param color CSS color string (e.g., "#ffffff", "red", "rgb(255,0,0)")
        void SetBackgroundColor(const std::string& color) noexcept;

        /// Sets the border color.
        /// @param color CSS color string
        void SetBorderColor(const std::string& color) noexcept;

        /// Sets the border width in pixels.
        /// @param width Border width (must be >= 0)
        void SetBorderWidth(int width) noexcept;

        /// Sets the border radius in pixels for rounded corners.
        /// @param radius Radius in pixels (must be >= 0)
        void SetBorderRadius(int radius) noexcept;

        /// Sets the padding in pixels.
        /// @param padding Padding size (must be >= 0)
        void SetPadding(int padding) noexcept;

        /// Sets the margin in pixels.
        /// @param margin Margin size (must be >= 0)
        void SetMargin(int margin) noexcept;

        /// Sets the width in pixels.
        /// @param width Width in pixels (must be > 0)
        void SetWidth(int width) noexcept;

        /// Sets the height in pixels.
        /// @param height Height in pixels (must be > 0)
        void SetHeight(int height) noexcept;

        /// Sets the opacity/transparency level.
        /// @param opacity Opacity value from 0.0 (transparent) to 1.0 (opaque)
        void SetOpacity(double opacity) noexcept;

        /// Sets the box shadow CSS property.
        /// @param shadow CSS box-shadow string (e.g., "0 4px 6px rgba(0,0,0,0.1)")
        void SetBoxShadow(const std::string& shadow) noexcept;

        /// Toggles the visibility of this layout.
        void ToggleVisibility() noexcept;

        /// @brief check if the layout is visible
        /// @return true if visible, false if hidden
        bool IsVisible() const noexcept { return mIsVisible; }

        // ===== DOM Synchronization =====

        /// Applies all layout configuration and styling to the DOM.
        /// This method synchronizes all stored style properties and layout parameters
        /// with the actual DOM element and its children. Call after making changes.
        void Apply() noexcept;

        /// Clears all child elements from the layout and removes them from the DOM.
        void Clear() noexcept;

    private:
        // ===== DOM References =====
        emscripten::val mDocument; ///< Reference to the document object

        // ===== Element Tracking =====
        std::vector<IDomElement*> mChildren{}; ///< Pointers to child elements
        std::unordered_map<IDomElement*, Alignment> mParams{}; ///< Alignment per child

        // ===== Layout State =====
        LayoutType mType = LayoutType::Free; ///< Current layout type
        Justification mJustification = Justification::Start; ///< Main axis alignment
        Alignment mAlignItems = Alignment::None; ///< Cross axis alignment

        // ===== Styling Properties =====
        std::string mBackgroundColor{};
        std::string mBorderColor{};
        std::optional<int> mSpacing{};
        std::optional<int> mBorderWidth{};
        std::optional<int> mBorderRadius{};
        std::optional<int> mPadding{};
        std::optional<int> mMargin{};
        std::optional<int> mWidth{};
        std::optional<int> mHeight{};
        std::optional<double> mOpacity{};
        std::string mBoxShadow{};
        bool mIsVisible = true;

        static int mNextIdCounter; ///< Counter for auto-generated element IDs

        /// Apply styling options to root layout
        /// @param style the style object for the root layout
        void ApplyStyling(emscripten::val style) noexcept;

        /// Apply layout options to root layout
        /// @param style the style object for the root layout
        void ApplyLayout(emscripten::val style) noexcept;

        /// Apply children style and layout
        void ApplyChildren() noexcept;
    };

} // namespace cse498
