#pragma once

#include <memory>
#include <string>
#include <vector>

#include "../internal/IDomElement.hpp"
#include "../internal/ICanvasElement.hpp"

namespace cse498 {

/**
 * WebCanvas
 * - As an IDomElement: can be mounted/unmounted/synced by WebLayout (DOM lifecycle).
 * - As a canvas manager: owns ICanvasElement objects and dispatches Draw() in RenderFrame().
 *
 * Additionally, WebCanvas exposes a small set of immediate-mode primitives
 * (line/circle/point/polygon) to support "whiteboard-level" drawing and demos.
 */
class WebCanvas : public IDomElement {
public:
    struct Vec2 { float x; float y; };

    explicit WebCanvas(std::string id = "web-canvas");
    ~WebCanvas() override = default;

    // Move-only (RAII-friendly).
    WebCanvas(const WebCanvas&) = delete;
    WebCanvas& operator=(const WebCanvas&) = delete;
    WebCanvas(WebCanvas&&) noexcept = default;
    WebCanvas& operator=(WebCanvas&&) noexcept = default;

    // ---- IDomElement ----
    void MountToLayout(WebLayout& parent, Alignment align = Alignment::Start) override;
    void Unmount() override;
    void SyncFromModel() override;
    const std::string& Id() const override { return mId; }

    // ---- Canvas content management ----
    void AddElement(std::unique_ptr<ICanvasElement> element);
    void ClearElements();
    void RenderFrame();

    // ---- Immediate-mode primitive drawing (whiteboard-level) ----
    // Notes:
    // - In non-Emscripten builds these are safe no-ops (useful for unit tests).
    // - In Emscripten builds these issue Canvas2D calls on the <canvas id="Id()">.
    // void Clear(const std::string& cssColor = "#000000");
    void Clear(const std::string& cssColor);

    void DrawLine(float x1, float y1, float x2, float y2,
                  float lineWidth, // = 1.0f,
                  const std::string& strokeColor); // = "#ffffff");

    // If fillColor is empty => no fill.
    void DrawCircle(float centerX, float centerY, float radius,
                    const std::string& strokeColor, // = "#ffffff",
                    float lineWidth, // = 1.0f,
                    const std::string& fillColor); // = "");

    void DrawPoint(float x, float y,
                   float radius, // = 2.0f,
                   const std::string& fillColor); // = "#ffffff");

    // If fillColor is empty => no fill.
    void DrawPolygon(const std::vector<Vec2>& points,
                     const std::string& strokeColor, // = "#ffffff",
                     float lineWidth, // = 1.0f,
                     const std::string& fillColor); // = "");

private:
    std::vector<std::unique_ptr<ICanvasElement>> m_elements;

    // DOM-side state (minimal early-stage implementation)
    std::string mId;
    WebLayout*  mParent  = nullptr;       // Non-owning pointer to the parent layout
    Alignment   mAlign   = Alignment::Start;
    bool        mMounted = false;
};

} // namespace cse498