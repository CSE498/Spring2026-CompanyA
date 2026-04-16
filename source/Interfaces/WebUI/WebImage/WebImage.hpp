/**
 * @file WebImage.hpp
 * @brief Declaration of WebImage, a DOM-backed HTML \<img\> element for WebUI.
 *
 * WebImage manages an HTML \<img\> element from C++ via Emscripten. It
 * implements IDomElement for DOM lifecycle management (mount/unmount/sync)
 * and ICanvasElement for optional canvas-based rendering. Positioning is
 * NOT handled by WebImage; use WebLayout (Flex/Grid/Free) to control where
 * the image appears on the page.
 *
 */

#pragma once

#include <emscripten/val.h>
#include <functional>
#include <string>
#include <unordered_map>

#include "../../../tools/Color.hpp"
#include "../internal/ICanvasElement.hpp"
#include "../internal/IDomElement.hpp"

namespace cse498 {

    /// @brief Behavior when an image source fails to load.
    enum class ImageErrorMode {
        BlankRect, ///< Show a blank colored rectangle as placeholder.
        NoOp ///< Do nothing; the element remains invisible/broken.
    };

    /**
     * @class WebImage
     * @brief DOM-backed HTML \<img\> element for use in WebUI layouts.
     *
     * Manages an HTML \<img\> element from C++ via Emscripten.
     * Can be rendered as a DOM element via WebLayout or drawn on a WebCanvas.
     * Positioning is NOT handled by WebImage; use WebLayout (Flex/Grid/Free)
     * to control where the image appears on the page.
     */
    class WebImage : public IDomElement, public ICanvasElement {
    public:
        /// Construct a detached WebImage with a source URL/path and optional alt text.
        /// The underlying DOM element is created but not mounted; callers should
        /// attach it through WebLayout when they want it to appear in the document.
        explicit WebImage(const std::string& src, const std::string& alt_text = "");
        ~WebImage();

        WebImage(const WebImage&) = delete;
        WebImage& operator=(const WebImage&) = delete;

        WebImage(WebImage&& other) noexcept;
        WebImage& operator=(WebImage&& other) noexcept;

        // ----- Source & Metadata -----

        /// Set the image source (URL or asset path).
        void SetSource(const std::string& src);
        /// Get the current image source.
        [[nodiscard]] std::string GetSource() const;

        /// Set the alternative text for accessibility.
        void SetAltText(const std::string& alt_text);
        /// Get the alternative text.
        [[nodiscard]] std::string GetAltText() const;

        // ----- Sizing -----

        /// Set the display size of the image (does not preserve aspect ratio).
        /// @param width_px  Width in pixels (must be >= 0)
        /// @param height_px Height in pixels (must be >= 0)
        void SetSize(int width_px, int height_px);

        /// Resize the image. If maintain_aspect_ratio is true, the image is
        /// scaled to fit within the given bounding box while preserving its
        /// original aspect ratio (CSS object-fit: contain). Otherwise it
        /// stretches to fill the exact dimensions (CSS object-fit: fill).
        /// @param width_px              Target width in pixels (must be > 0)
        /// @param height_px             Target height in pixels (must be > 0)
        /// @param maintain_aspect_ratio If true, scale to fit; if false, stretch.
        void Resize(int width_px, int height_px, bool maintain_aspect_ratio = false);

        /// Get the display width.
        [[nodiscard]] int GetWidth() const;
        /// Get the display height.
        [[nodiscard]] int GetHeight() const;

        // ----- Opacity / Transparency -----

        /// Set the opacity of the image (0.0 = fully transparent, 1.0 = fully opaque).
        void SetOpacity(double alpha);
        /// Get the current opacity.
        [[nodiscard]] double GetOpacity() const;

        // ----- Visibility -----

        /// Show the image.
        void Show();
        /// Hide the image.
        void Hide();
        /// Check if the image is visible.
        [[nodiscard]] bool IsVisible() const;

        // ----- Loading State & Error Handling -----

        /// Check if the image has been loaded.
        [[nodiscard]] bool IsLoaded() const;
        /// Check if the image source failed to load.
        [[nodiscard]] bool HasError() const;

        /// Set a callback to be invoked when the image finishes loading.
        void SetOnLoadCallback(std::function<void()> callback);

        /// Set a callback to be invoked when the image fails to load.
        void SetOnErrorCallback(std::function<void()> callback);

        /// Set the behavior when an image fails to load.
        /// BlankRect: renders a colored placeholder rectangle.
        /// NoOp: does nothing (element stays as-is).
        void SetErrorMode(ImageErrorMode mode);

        /// Set the placeholder color shown when image fails to load
        /// (only used when error mode is BlankRect). Any valid CSS color.
        void SetPlaceholderColor(const std::string& css_color);

        // ----- IDomElement Interface -----

        void MountToLayout(WebLayout& parent, Alignment align) override;
        void SyncFromModel() override;

        // ----- ICanvasElement Interface -----

        /// Sets the position and optional size used when drawing this image on a
        /// WebCanvas via Draw(). Coordinates are in canvas pixel space.
        /// @param x Canvas x coordinate (left edge).
        /// @param y Canvas y coordinate (top edge).
        /// @param w Draw width in pixels; pass -1 to use the image's display width.
        /// @param h Draw height in pixels; pass -1 to use the image's display height.
        void SetCanvasRect(float x, float y, float w = -1.0f, float h = -1.0f);

        /// Returns the canvas x coordinate set by SetCanvasRect().
        [[nodiscard]] float CanvasX() const;
        /// Returns the canvas y coordinate set by SetCanvasRect().
        [[nodiscard]] float CanvasY() const;
        /// Returns the canvas draw width set by SetCanvasRect() (-1 = use display width).
        [[nodiscard]] float CanvasW() const;
        /// Returns the canvas draw height set by SetCanvasRect() (-1 = use display height).
        [[nodiscard]] float CanvasH() const;

        void Draw(WebCanvas& canvas) override;

        /// Handle load event (called when image finishes loading).
        void HandleLoad();
        /// Handle error event (called when image fails to load).
        void HandleError();

    public:
        /// @brief Looks up the WebImage registered under the given registry id.
        /// @param id Registry id to look up.
        /// @return Pointer to the registered WebImage, or nullptr if not found.
        [[nodiscard]] static WebImage* LookupImage(int id);

    private:
        friend struct WebImageTestAccessor;

        // ----- Image registry (maps integer IDs to WebImage pointers) -----
        static std::unordered_map<int, WebImage*> sImageRegistry;
        static int sNextRegistryId;

        static int RegisterImage(WebImage* img);
        static void UnregisterImage(int id);
        static void UpdateRegistryEntry(int id, WebImage* img);

        // ----- Style helper methods (eliminate repeated lambda definitions) -----
        static void ApplyDimensionStyle(emscripten::val& style, const char* prop, int value);
        static void ApplyPlaceholderDimensionStyle(emscripten::val& style, const char* prop, int value);

        std::string mSrc; ///< Image source URL or asset path.
        std::string mAltText; ///< Alternative text for accessibility.
        int mWidth = 0; ///< Display width in pixels (0 = browser default).
        int mHeight = 0; ///< Display height in pixels (0 = browser default).
        double mOpacity = 1.0; ///< Opacity in [0.0, 1.0].
        bool mIsVisible = true; ///< Whether the image element is visible.
        bool mIsLoaded = false; ///< True after the image finishes loading.
        bool mHasError = false; ///< True if the image source failed to load.
        ImageErrorMode mErrorMode = ImageErrorMode::BlankRect; ///< Error handling strategy.
        std::string mPlaceholderColor = Color::FromRGB255(204, 204, 204).ToHex();
        ///< Color used for the BlankRect placeholder.
        std::function<void()> mOnLoadCallback; ///< Callback invoked on successful load.
        std::function<void()> mOnErrorCallback; ///< Callback invoked on load failure.
        int mRegistryId = -1; ///< Registry id for JS event forwarding.

        float mCanvasX = 0.0f; ///< Canvas draw position x (pixels).
        float mCanvasY = 0.0f; ///< Canvas draw position y (pixels).
        float mCanvasW = -1.0f; ///< Canvas draw width (-1 = use mWidth).
        float mCanvasH = -1.0f; ///< Canvas draw height (-1 = use mHeight).

        static int mNextIdCounter; ///< Counter for generating unique DOM ids.

        /// Mark the image as loaded or not (for tracking async loading).
        void MarkLoaded(bool loaded);
        /// Remove the element from its parent DOM node.
        void RemoveFromDom();
        /// Remove from DOM, deregister from event registry, and null the element.
        void CleanupElement();
        /// Attach the onload/onerror event listeners to the DOM element.
        void AttachListeners();
        /// Apply the blank-rect placeholder fallback.
        void ApplyPlaceholder();
        /// Get the browser's document object.
        static emscripten::val GetDocument();
        /// Convert an integer to a CSS pixel string (e.g. "100px").
        static std::string ToPx(int value);
    };

} // namespace cse498
