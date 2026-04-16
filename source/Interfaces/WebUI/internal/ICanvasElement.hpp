/**
 * @file ICanvasElement.hpp
 * @brief Interface for elements that can be drawn onto a WebCanvas.
 *
 * Defines the ICanvasElement base interface that any drawable object must
 * implement to participate in WebCanvas's render pipeline. Elements are
 * ordered by z-index and can be individually shown or hidden.
 *
 */

#pragma once

namespace cse498 {

    class WebCanvas;

    /**
     * @class ICanvasElement
     * @brief Abstract base for objects renderable by WebCanvas.
     *
     * Classes that implement this interface can be added to a WebCanvas and
     * will have their Draw() method called each render frame in z-index order.
     * The Metadata struct tracks per-element z-index and visibility state.
     */
    class ICanvasElement {
    public:
        virtual ~ICanvasElement() = default;

        /// @brief Draws this element onto the given canvas.
        /// @param canvas The WebCanvas to draw onto.
        virtual void Draw(WebCanvas& canvas) = 0;

        /// @brief Returns the z-index used for draw ordering.
        /// @return Current z-index value; lower values are drawn first.
        int ZIndex() const { return mMetadata.mZIndex; }

        /// @brief Returns whether this element is currently visible.
        /// @return True if the element should be drawn; false if skipped.
        bool Visible() const { return mMetadata.mIsVisible; }

        /// @brief Sets the z-index used for draw ordering.
        /// @param z New z-index value.
        void SetZIndex(int z) { mMetadata.mZIndex = z; }

        /// @brief Sets the visibility of this element.
        /// @param v True to show, false to hide.
        void SetVisible(bool v) { mMetadata.mIsVisible = v; }

    protected:
        /// @brief Per-element metadata used by the canvas scheduler.
        struct Metadata {
            int mZIndex = 0; ///< Draw order index; lower values drawn first.
            bool mIsVisible = true; ///< Whether the element participates in rendering.
        };

        Metadata mMetadata; ///< Scheduling metadata for this canvas element.
    };

} // namespace cse498
