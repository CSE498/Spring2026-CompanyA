#pragma once

#include <string>

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
 /// @param align Optional alignment parameter for positioning within the parent
 virtual void mountToLayout(WebLayout& parent,
                            Alignment align = Alignment::None) = 0;

 /// Unmounts this element from its current parent layout.
 virtual void unmount() = 0;

 /// Synchronizes this element's state with the DOM.
 /// Called when the element model has changed and needs to be reflected in the
 /// DOM.
 virtual void syncFromModel() = 0;

 /// Returns the unique identifier of this DOM element.
 /// @return Const reference to the element's unique ID string
 virtual const std::string& Id() const = 0;

 // ===== Grid Layout Support =====

 /// Sets the grid row and column position for this element in a grid layout.
 /// Use -1 to indicate "unset" (will default to auto-placement).
 /// @param row The grid row index (0-based)
 /// @param col The grid column index (0-based)
 void SetGridPosition(int row, int col) {
   grid_row_ = row;
   grid_col_ = col;
 }

 /// Returns the grid row position for this element.
 /// @return Grid row index, or -1 if not set
 int GridRow() const { return grid_row_; }

 /// Returns the grid column position for this element.
 /// @return Grid column index, or -1 if not set
 int GridCol() const { return grid_col_; }

 /// Clears the grid position, allowing auto-placement in parent grid.
 void ClearGridPosition() { grid_row_ = grid_col_ = -1; }

 // ===== Free Layout Support (Absolute/Relative Positioning) =====

 /// Sets the offset position (top and left) for this element in free layout.
 /// Used when parent layout is LayoutType::Free.
 /// @param top Vertical offset in pixels, or -1 to leave unset
 /// @param left Horizontal offset in pixels, or -1 to leave unset
 void SetFreePosition(int top, int left) {
   top_ = top;
   left_ = left;
 }

 /// Returns the top offset for this element in free layout.
 /// @return Top offset in pixels, or -1 if not set
 int FreeTop() const { return top_; }

 /// Returns the left offset for this element in free layout.
 /// @return Left offset in pixels, or -1 if not set
 int FreeLeft() const { return left_; }

 /// Clears the free position offsets for this element.
 void ClearFreePosition() { top_ = left_ = -1; }

private:
 int grid_row_ = -1;  ///< Grid row position (-1 = unset)
 int grid_col_ = -1;  ///< Grid column position (-1 = unset)
 int top_ = -1;       ///< Free layout top offset in pixels (-1 = unset)
 int left_ = -1;      ///< Free layout left offset in pixels (-1 = unset)
};
