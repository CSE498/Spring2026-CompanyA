/**
 * @file IDomElement.hpp
 * @brief Interface for DOM-backed UI elements managed by WebLayout.
 *
 * Defines the IDomElement base interface that all WebUI components must
 * implement in order to participate in WebLayout's lifecycle management
 * (mounting, unmounting, and model synchronization). Also provides grid
 * and free-layout positioning helpers.
 *
 */

#pragma once

#include <emscripten/val.h>
#include <string>

#include "../WebUtils.hpp"

namespace cse498 {

/// Enum defining alignment values for positioning elements along one axis.
/// - None: No specific alignment
/// - Start: Align to the start of the axis (flex-start)
/// - Center: Center align
/// - End: Align to the end of the axis (flex-end)
/// - Stretch: Stretch to fill available space
enum class Alignment { None, Start, Center, End, Stretch };

class WebLayout;

/// Interface for DOM elements that can be managed by WebLayout containers.
/// All components that can be added to a WebLayout must implement this
/// interface. It provides methods for integration with layout management and
/// state synchronization.
class IDomElement {
public:
    virtual ~IDomElement() = default;

    /// Mounts this element into a parent layout with optional alignment.
    /// @param parent Reference to the parent WebLayout container
    /// @param align Optional alignment parameter for positioning within the
    /// parent
    virtual void MountToLayout(WebLayout& parent, Alignment align = Alignment::None) = 0;

    /// @brief Removes this element from its current DOM parent.
    virtual void Unmount() {
        if (mElement.isNull())
            return;

        if (mParent != nullptr) {
            mParent->RemoveChild(this);
            mParent = nullptr;
        }
    }

    /// Synchronizes this element's state with the DOM.
    /// Called when the element model has changed and needs to be reflected in the
    /// DOM.
    virtual void SyncFromModel() = 0;

    /// Returns the unique identifier of this DOM element.
    /// @return Const reference to the element's unique ID string
    [[nodiscard]] const std::string& Id() const { return mId; };

    /// @brief Get a reference to DOM Element for this element
    /// @return a reference to DOM Element for this element
    [[nodiscard]] emscripten::val GetElement() { return mElement; }

    /// @brief Get a pointer to the parent element of this element
    /// @return a pointer to the parent element of this element or nullptr if no parent
    [[nodiscard]] IDomElement* GetParent() { return mParent; }

    /// @brief Set the parent of this element
    /// @param parent a pointer to the parent element
    void SetParent(IDomElement* parent) { mParent = parent; }

    /// @brief Remove the specified child element from the DOM.
    ///        Can be overridden to allow side effects.
    /// @param element a pointer to the child element to remove
    virtual void RemoveChild(IDomElement* element) {
        if (mElement.isNull() && mElement.isUndefined()) {
            GetConsole().call<void>("warn", "element with Id: " + Id() + " is undefined");
            return;
        };

        auto parent = element->GetParent();

        if (!parent) {
            return;
        }

        auto parentElement = parent->GetElement();

        if (parentElement.isNull() || parentElement.isUndefined()) {
            // clang-format off
            GetConsole().call<void>("warn", string("parent is not nullptr but parent element is undefined"));
            // clang-format on
            return;
        }

        if (parent->GetElement() != mElement) {
            // clang-format off
            GetConsole().call<void>("warn", "element with Id: " + element->Id() + " is not a child of element with Id: " + Id());
            // clang-format on
            return;
        }

        if (element->mExisting) {
            // clang-format off
            GetConsole().call<void>("warn", "element with Id: " + element->Id() + " is an existing element and will not be removed.");
            // clang-format on
            return;
        }

        mElement.call<void>("removeChild", element->GetElement());
    }

    // ===== Grid Layout Support =====

    /// Sets the grid row and column position for this element in a grid layout.
    /// Use -1 to indicate "unset" (will default to auto-placement).
    /// @param row The grid row index (0-based)
    /// @param col The grid column index (0-based)
    void SetGridPosition(int row, int col) {
        mGridRow = row;
        mGridCol = col;
    }

    /// Returns the grid row position for this element.
    /// @return Grid row index, or -1 if not set
    [[nodiscard]] int GridRow() const { return mGridRow; }

    /// Returns the grid column position for this element.
    /// @return Grid column index, or -1 if not set
    [[nodiscard]] int GridCol() const { return mGridCol; }

    /// Clears the grid position, allowing auto-placement in parent grid.
    void ClearGridPosition() { mGridRow = mGridCol = -1; }

    // ===== Free Layout Support (Absolute/Relative Positioning) =====

    /// Sets the offset position (top and left) for this element in free layout.
    /// Used when parent layout is LayoutType::Free.
    /// @param top Vertical offset in pixels, or -1 to leave unset
    /// @param left Horizontal offset in pixels, or -1 to leave unset
    void SetFreePosition(int top, int left) {
        mTop = top;
        mLeft = left;
    }

    /// Returns the top offset for this element in free layout.
    /// @return Top offset in pixels, or -1 if not set
    [[nodiscard]] int FreeTop() const { return mTop; }

    /// Returns the left offset for this element in free layout.
    /// @return Left offset in pixels, or -1 if not set
    [[nodiscard]] int FreeLeft() const { return mLeft; }

    /// Clears the free position offsets for this element.
    void ClearFreePosition() { mTop = mLeft = -1; }

protected:
    emscripten::val mElement = emscripten::val::undefined(); ///< HTML element for this dom element
    IDomElement* mParent = nullptr; ///< DOM element for the parent of this element
    std::string mId{}; ///< DOM Id of this element
    bool mExisting{false}; ///< True if this element is just hooking into an existing element

    int mGridRow = -1; ///< Grid row position (-1 = unset)
    int mGridCol = -1; ///< Grid column position (-1 = unset)
    int mTop = -1; ///< Free layout top offset in pixels (-1 = unset)
    int mLeft = -1; ///< Free layout left offset in pixels (-1 = unset)
};

} // namespace cse498
