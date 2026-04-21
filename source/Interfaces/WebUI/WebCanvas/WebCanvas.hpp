/**
 * @file WebCanvas.hpp
 * @brief Declaration of WebCanvas, a DOM-backed 2D drawing surface for WebUI.
 *
 * WebCanvas serves two roles:
 *  - As an IDomElement it participates in WebLayout's DOM lifecycle
 *    (mount, unmount, sync).
 *  - As a canvas manager it owns ICanvasElement objects and dispatches
 *    their Draw() calls each frame via RenderFrame().
 *
 * A small set of immediate-mode primitives (line, circle, point, polygon)
 * is also exposed for whiteboard-level drawing and demos.
 *
 */

#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../internal/ICanvasElement.hpp"
#include "../internal/IDomElement.hpp"

namespace cse498 {

/**
 * @class WebCanvas
 * @brief 2D drawing surface backed by an HTML \<canvas\> element.
 *
 * As an IDomElement, WebCanvas can be mounted into and managed by a
 * WebLayout container. As a canvas manager it maintains a sorted list of
 * ICanvasElement objects and renders them each frame in z-index order.
 * Immediate-mode drawing primitives are also available for simple demos;
 * in non-Emscripten builds these are safe no-ops.
 */
class WebCanvas : public IDomElement {
public:
    /// @brief 2D floating-point vector used for polygon vertex coordinates.
    struct Vec2 {
        float x;
        float y;
    };

    /// @brief Constructs a WebCanvas associated with the given canvas element id.
    /// @param id DOM id of the \<canvas\> element (default: "web-canvas").
    explicit WebCanvas(const std::string& id = "web-canvas");

    ~WebCanvas() override = default;

    // Move-only (RAII-friendly).
    WebCanvas(const WebCanvas&) = delete;
    WebCanvas& operator=(const WebCanvas&) = delete;
    WebCanvas(WebCanvas&&) noexcept = default;
    WebCanvas& operator=(WebCanvas&&) noexcept = default;

    /// @brief set the color used to clear the screen before drawing
    /// @param color the color to set
    void SetBackgroundColor(const std::string& color) { mBackgroundColor = color; }

    // ---- IDomElement ----

    /// @brief Mounts this canvas into the given parent layout.
    /// @param parent Parent WebLayout to attach to.
    /// @param align  Alignment within the parent (default: Start).
    void MountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;

    /// @brief Unmounts this canvas from its parent layout.
    void Unmount() override;

    /// @brief Synchronizes canvas state with the DOM by calling RenderFrame().
    void SyncFromModel() override;

    // ---- Canvas content management ----

    /// @brief Adds a drawable ICanvasElement to the render list.
    /// @param element Unique pointer to the element; must not be null.
    void AddElement(std::unique_ptr<ICanvasElement> element);

    /// @brief Removes all ICanvasElement objects from the render list.
    void ClearElements();

    /// @brief Renders one frame: sorts elements by z-index and calls Draw() on each visible one.
    void RenderFrame();

    // ---- Immediate-mode primitive drawing (whiteboard-level) ----
    // Notes:
    // - In non-Emscripten builds these are safe no-ops (useful for unit tests).
    // - In Emscripten builds these issue Canvas2D calls on the <canvas id="Id()">.

    /// @brief Clears the canvas with the given CSS background color.
    /// @param cssColor CSS color string (e.g., "#000000").
    void Clear(const std::string& cssColor = "");

    /// @brief Draws a line between two points.
    /// @param x1          Start x coordinate.
    /// @param y1          Start y coordinate.
    /// @param x2          End x coordinate.
    /// @param y2          End y coordinate.
    /// @param lineWidth   Stroke width in pixels.
    /// @param strokeColor CSS stroke color string.
    void DrawLine(float x1, float y1, float x2, float y2, float lineWidth, const std::string& strokeColor);

    /// @brief Draws a circle.
    /// @param centerX     Center x coordinate.
    /// @param centerY     Center y coordinate.
    /// @param radius      Circle radius in pixels.
    /// @param strokeColor CSS stroke color string.
    /// @param lineWidth   Stroke width in pixels.
    /// @param fillColor   CSS fill color string; empty string means no fill.
    void DrawCircle(float centerX, float centerY, float radius, const std::string& strokeColor, float lineWidth,
                    const std::string& fillColor);

    /// @brief Draws a filled point (small circle).
    /// @param x         Center x coordinate.
    /// @param y         Center y coordinate.
    /// @param radius    Point radius in pixels.
    /// @param fillColor CSS fill color string.
    void DrawPoint(float x, float y, float radius, const std::string& fillColor);

    /// @brief Draws a polygon defined by a list of vertices.
    /// @param points      Ordered list of vertex positions.
    /// @param strokeColor CSS stroke color string.
    /// @param lineWidth   Stroke width in pixels.
    /// @param fillColor   CSS fill color string; empty string means no fill.
    void DrawPolygon(const std::vector<Vec2>& points, const std::string& strokeColor, float lineWidth,
                     const std::string& fillColor);

    /// @brief Draws a filled rectangle.
    /// @param x         Left edge x coordinate.
    /// @param y         Top edge y coordinate.
    /// @param w         Width in pixels.
    /// @param h         Height in pixels.
    /// @param fillColor CSS fill color string.
    void DrawRect(float x, float y, float w, float h, const std::string& fillColor);

    /// @brief Draws a text string onto the canvas.
    /// @param x        Left edge x coordinate of the text baseline.
    /// @param y        Baseline y coordinate.
    /// @param text     UTF-8 string to render.
    /// @param color    CSS fill color string for the text.
    /// @param fontSize Font size in pixels.
    /// @param fontFamily Name of font to use
    void DrawText(float x, float y, const std::string& text, const std::string& color, float fontSize,
                  const std::string& fontFamily);

    /// @brief Draws an image (by source URL) onto the canvas.
    /// @param imgSrc  URL or asset path of the image to draw.
    /// @param x       Left edge x coordinate.
    /// @param y       Top edge y coordinate.
    /// @param w       Width in pixels; pass -1 to use the image's natural width.
    /// @param h       Height in pixels; pass -1 to use the image's natural height.
    void DrawImage(const std::string& imgSrc, float x, float y, float w, float h);

    /// @brief Draws a bitmap image to the canvas
    /// @param bitmapHandle handle pointing to the bitmap object
    /// @param x            the x-coordinate of the bottom center of the image
    /// @param y            the y-coordinate of the bottom center of the image
    /// @param scale        the scale factor to apply to the image before drawing
    void DrawTexture(emscripten::EM_VAL bitmapHandle, int x, int y, double scale);

private:
    std::vector<std::unique_ptr<ICanvasElement>> mElements; ///< Owned canvas elements.
    std::string mBackgroundColor{};

    Alignment mAlign = Alignment::Start; ///< Alignment within the parent layout.
    bool mMounted = false; ///< Whether this canvas is currently mounted.
    bool mExisting = false; ///< Does the actual DOM <canvas> element exist

};

} // namespace cse498
