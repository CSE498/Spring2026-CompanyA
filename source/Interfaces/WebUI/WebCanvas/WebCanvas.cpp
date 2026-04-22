/**
 * @file WebCanvas.cpp
 * @brief Implementation of WebCanvas DOM-backed 2D drawing surface.
 *
 * Implements IDomElement lifecycle (mount/unmount/sync) and ICanvasElement
 * render dispatch. Also implements immediate-mode drawing primitives that
 * delegate to JavaScript canvas helper functions when compiled with
 * Emscripten, and are safe no-ops in native builds.
 *
 */

#include "WebCanvas.hpp"
#include "../../../tools/Color.hpp"

#include <algorithm> // std::stable_sort
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include "../WebErrorManager/WebErrorManager.hpp"
#include "../WebLayout/WebLayout.hpp"
#endif

namespace cse498 {

// --------------------
// External JS library functions (from WebCanvas.js via --js-library)
// --------------------
#ifdef __EMSCRIPTEN__
extern "C" {
void webcanvas__clear(const char* id, const char* cssColor);
void webcanvas__draw_line(const char* id, float x1, float y1, float x2, float y2, float lineWidth,
                          const char* strokeColor);
void webcanvas__draw_circle(const char* id, float cx, float cy, float r, const char* strokeColor, float lineWidth,
                            const char* fillColor);
void webcanvas__draw_polygon(const char* id, const float* coordsXY, int count, const char* strokeColor, float lineWidth,
                             const char* fillColor);
void webcanvas__draw_rect(const char* id, float x, float y, float w, float h, const char* fillColor);
void webcanvas__fill_text(const char* id, float x, float y, const char* text, const char* color, float fontSize,
                          const char* fontFamily);
void webcanvas__draw_image(const char* id, const char* imgSrc, float x, float y, float w, float h);
void webcanvas__init(const char* id);
void webcanvas__draw_texture(const char* id, emscripten::EM_VAL bitmapHandle, int x, int y, double scale);
}
#endif

// --------------------
// WebCanvas
// --------------------

/// @brief Constructs a WebCanvas with the given canvas element id.
/// @param id DOM id of the \<canvas\> element; defaults to "web-canvas" if empty.
WebCanvas::WebCanvas(const std::string& id) {
    mId = id;
    if (mId.empty()) {
        mId = "web-canvas";
    }
#ifdef __EMSCRIPTEN__
    mElement = GetDocument().call<emscripten::val>("getElementById", mId);
    mAdoptsExistingDom = !mElement.isNull() && !mElement.isUndefined();

    if (!mAdoptsExistingDom) {
        WebErrorManager::Options options;
        options.showAlert = true;

        WebErrorManager::Error("WebCanvas error: required canvas element with id '" + mId + "' was not found.",
                               options);
        return;
    }

    webcanvas__init(mId.c_str());
#endif
}

// ---- IDomElement ----

/// @brief Records the parent layout and marks this canvas as mounted.
/// @param parent Parent WebLayout to attach to.
/// @param align  Alignment within the parent.
void WebCanvas::MountToLayout(WebLayout& parent, Alignment align) {
#ifdef __EMSCRIPTEN__
    parent.AddElement(this, align);
#endif
    mAlign = align;
    mMounted = true;
}

/// @brief Clears the parent reference and marks this canvas as unmounted.
void WebCanvas::Unmount() {
#ifdef __EMSCRIPTEN__
    IDomElement::Unmount();
#endif
    mMounted = false;
}

/// @brief calls RenderFrame so the canvas is updated when its parent wants to update the screen
void WebCanvas::SyncFromModel() { RenderFrame(); }

// ---- Canvas content management ----

/// @brief Adds a drawable element to the render list.
/// @param element Unique pointer to the element; null pointers are ignored.
void WebCanvas::AddElement(std::unique_ptr<ICanvasElement> element) {
    if (!element) {
        return;
    }
    mElements.emplace_back(std::move(element));
}

/// @brief Removes all ICanvasElement objects from the render list.
void WebCanvas::ClearElements() { mElements.clear(); }

/// @brief Renders one frame: sorts elements by z-index (stable) and calls
///        Draw() on each visible element in ascending z-index order.
void WebCanvas::RenderFrame() {
    // Collect raw pointers for stable sorting without moving ownership.
    std::vector<ICanvasElement*> ordered;
    ordered.reserve(mElements.size());
    for (auto& e: mElements) {
        ordered.push_back(e.get());
    }

    // Stable sort by zIndex (ascending). Elements with the same zIndex keep insertion order.
    std::stable_sort(ordered.begin(), ordered.end(),
                     [](const ICanvasElement* a, const ICanvasElement* b) { return a->ZIndex() < b->ZIndex(); });

    // Draw visible elements only.
    for (auto* e: ordered) {
        if (!e->Visible()) {
            continue;
        }
        e->Draw(*this);
    }
}

// ---- Immediate-mode primitives ----

/// @brief Clears the canvas with the given CSS background color.
/// @param cssColor CSS color string (e.g., "#000000").
void WebCanvas::Clear(const std::string& cssColor) {
#ifdef __EMSCRIPTEN__
    webcanvas__clear(mId.c_str(), cssColor.empty() ? mBackgroundColor.c_str() : cssColor.c_str());
#else
    (void) cssColor;
#endif
}

/// @brief Draws a line between two points on the canvas.
/// @param x1          Start x coordinate.
/// @param y1          Start y coordinate.
/// @param x2          End x coordinate.
/// @param y2          End y coordinate.
/// @param lineWidth   Stroke width in pixels.
/// @param strokeColor CSS stroke color string.
void WebCanvas::DrawLine(float x1, float y1, float x2, float y2, float lineWidth, const std::string& strokeColor) {
#ifdef __EMSCRIPTEN__
    webcanvas__draw_line(mId.c_str(), x1, y1, x2, y2, lineWidth, strokeColor.c_str());
#else
    (void) x1;
    (void) y1;
    (void) x2;
    (void) y2;
    (void) lineWidth;
    (void) strokeColor;
#endif
}

/// @brief Draws a circle on the canvas.
/// @param centerX     Center x coordinate.
/// @param centerY     Center y coordinate.
/// @param radius      Circle radius in pixels.
/// @param strokeColor CSS stroke color string.
/// @param lineWidth   Stroke width in pixels.
/// @param fillColor   CSS fill color; empty string means no fill.
void WebCanvas::DrawCircle(float centerX, float centerY, float radius, const std::string& strokeColor, float lineWidth,
                           const std::string& fillColor) {
#ifdef __EMSCRIPTEN__
    webcanvas__draw_circle(mId.c_str(), centerX, centerY, radius, strokeColor.c_str(), lineWidth, fillColor.c_str());
#else
    (void) centerX;
    (void) centerY;
    (void) radius;
    (void) strokeColor;
    (void) lineWidth;
    (void) fillColor;
#endif
}

/// @brief Draws a filled point (small circle with no stroke).
/// @param x         Center x coordinate.
/// @param y         Center y coordinate.
/// @param radius    Point radius in pixels.
/// @param fillColor CSS fill color string.
void WebCanvas::DrawPoint(float x, float y, float radius, const std::string& fillColor) {
    // Treat point as a filled circle with no stroke.
    DrawCircle(x, y, radius,
               /*stroke*/ Color::FromRGB255(0, 0, 0).ToHex(),
               /*lineWidth*/ 0.0f, fillColor);
}

/// @brief Draws a polygon defined by an ordered list of vertices.
/// @param points      Vertex positions; requires at least 2 points.
/// @param strokeColor CSS stroke color string.
/// @param lineWidth   Stroke width in pixels.
/// @param fillColor   CSS fill color; empty string means no fill.
void WebCanvas::DrawPolygon(const std::vector<Vec2>& points, const std::string& strokeColor, float lineWidth,
                            const std::string& fillColor) {
    if (points.size() < 2) {
        return;
    }

#ifdef __EMSCRIPTEN__
    std::vector<float> coords;
    coords.reserve(points.size() * 2);
    for (const auto& p: points) {
        coords.push_back(p.x);
        coords.push_back(p.y);
    }

    webcanvas__draw_polygon(mId.c_str(), coords.data(), static_cast<int>(points.size()), strokeColor.c_str(), lineWidth,
                            fillColor.c_str());
#else
    (void) points;
    (void) strokeColor;
    (void) lineWidth;
    (void) fillColor;
#endif
}

/// @brief Draws a filled rectangle on the canvas.
/// @param x         Left edge x coordinate.
/// @param y         Top edge y coordinate.
/// @param w         Width in pixels.
/// @param h         Height in pixels.
/// @param fillColor CSS fill color string.
void WebCanvas::DrawRect(float x, float y, float w, float h, const std::string& fillColor) {
#ifdef __EMSCRIPTEN__
    webcanvas__draw_rect(mId.c_str(), x, y, w, h, fillColor.c_str());
#else
    (void) x;
    (void) y;
    (void) w;
    (void) h;
    (void) fillColor;
#endif
}

/// @brief Draws a text string onto the canvas.
/// @param x        Left edge x coordinate.
/// @param y        Baseline y coordinate.
/// @param text     UTF-8 string to render.
/// @param color    CSS fill color string for the text.
/// @param fontSize Font size in pixels.
void WebCanvas::DrawText(float x, float y, const std::string& text, const std::string& color, float fontSize,
                         const std::string& fontFamily) {
#ifdef __EMSCRIPTEN__
    webcanvas__fill_text(mId.c_str(), x, y, text.c_str(), color.c_str(), fontSize, fontFamily.c_str());
#else
    (void) x;
    (void) y;
    (void) text;
    (void) color;
    (void) fontSize;
    (void) fontFamily;
#endif
}

/// @brief Draws an image (by source URL) onto the canvas.
/// @param imgSrc URL or asset path of the image.
/// @param x      Left edge x coordinate.
/// @param y      Top edge y coordinate.
/// @param w      Width in pixels; -1 uses the image's natural width.
/// @param h      Height in pixels; -1 uses the image's natural height.
void WebCanvas::DrawImage(const std::string& imgSrc, float x, float y, float w, float h) {
#ifdef __EMSCRIPTEN__
    webcanvas__draw_image(mId.c_str(), imgSrc.c_str(), x, y, w, h);
#else
    (void) imgSrc;
    (void) x;
    (void) y;
    (void) w;
    (void) h;
#endif
}

void WebCanvas::DrawTexture(emscripten::EM_VAL bitmapHandle, int x, int y, double scale) {
#ifdef __EMSCRIPTEN__
    webcanvas__draw_texture(mId.c_str(), bitmapHandle, x, y, scale);
#else
    (void) bitmap;
    (void) x;
    (void) y;
    (void) scale;
#endif
}

} // namespace cse498
