#include "WebCanvas.hpp"

#include <algorithm> // std::stable_sort
#include <utility>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

namespace cse498 {

// --------------------
// External JS library functions (from WebCanvas.js via --js-library)
// --------------------
#ifdef __EMSCRIPTEN__
extern "C" {
void webcanvas__clear(const char *id, const char *cssColor);
void webcanvas__draw_line(const char *id, float x1, float y1, float x2,
                          float y2, float lineWidth, const char *strokeColor);
void webcanvas__draw_circle(const char *id, float cx, float cy, float r,
                            const char *strokeColor, float lineWidth,
                            const char *fillColor);
void webcanvas__draw_polygon(const char *id, const float *coordsXY, int count,
                             const char *strokeColor, float lineWidth,
                             const char *fillColor);
}
#endif

// --------------------
// WebCanvas
// --------------------

WebCanvas::WebCanvas(std::string id) : mId(std::move(id)) {
  if (mId.empty()) {
    mId = "web-canvas";
  }
}

// ---- IDomElement ----
void WebCanvas::MountToLayout(WebLayout &parent, Alignment align) {
  mParent = &parent;
  mAlign = align;
  mMounted = true;
}

void WebCanvas::Unmount() {
  mParent = nullptr;
  mMounted = false;
}

void WebCanvas::SyncFromModel() {
  // Safe no-op placeholder for now.
}

// ---- Canvas content management ----
void WebCanvas::AddElement(std::unique_ptr<ICanvasElement> element) {
  if (!element) {
    return;
  }
  m_elements.emplace_back(std::move(element));
}

void WebCanvas::ClearElements() { m_elements.clear(); }

void WebCanvas::RenderFrame() {
  // Collect raw pointers for stable sorting without moving ownership.
  std::vector<ICanvasElement *> ordered;
  ordered.reserve(m_elements.size());
  for (auto &e : m_elements) {
    ordered.push_back(e.get());
  }

  // Stable sort by zIndex (ascending). Elements with the same zIndex keep
  // insertion order.
  std::stable_sort(ordered.begin(), ordered.end(),
                   [](const ICanvasElement *a, const ICanvasElement *b) {
                     return a->ZIndex() < b->ZIndex();
                   });

  // Draw visible elements only.
  for (auto *e : ordered) {
    if (!e->Visible()) {
      continue;
    }
    e->Draw(*this);
  }
}

// ---- Immediate-mode primitives ----
void WebCanvas::Clear(const std::string &cssColor) {
#ifdef __EMSCRIPTEN__
  webcanvas__clear(mId.c_str(), cssColor.c_str());
#else
  (void)cssColor;
#endif
}

void WebCanvas::DrawLine(float x1, float y1, float x2, float y2,
                         float lineWidth, const std::string &strokeColor) {
#ifdef __EMSCRIPTEN__
  webcanvas__draw_line(mId.c_str(), x1, y1, x2, y2, lineWidth,
                       strokeColor.c_str());
#else
  (void)x1;
  (void)y1;
  (void)x2;
  (void)y2;
  (void)lineWidth;
  (void)strokeColor;
#endif
}

void WebCanvas::DrawCircle(float centerX, float centerY, float radius,
                           const std::string &strokeColor, float lineWidth,
                           const std::string &fillColor) {
#ifdef __EMSCRIPTEN__
  webcanvas__draw_circle(mId.c_str(), centerX, centerY, radius,
                         strokeColor.c_str(), lineWidth, fillColor.c_str());
#else
  (void)centerX;
  (void)centerY;
  (void)radius;
  (void)strokeColor;
  (void)lineWidth;
  (void)fillColor;
#endif
}

void WebCanvas::DrawPoint(float x, float y, float radius,
                          const std::string &fillColor) {
  // Treat point as a filled circle with no stroke.
  DrawCircle(x, y, radius, /*stroke*/ "#000000", /*lineWidth*/ 0.0f, fillColor);
}

void WebCanvas::DrawPolygon(const std::vector<Vec2> &points,
                            const std::string &strokeColor, float lineWidth,
                            const std::string &fillColor) {
  if (points.size() < 2) {
    return;
  }

#ifdef __EMSCRIPTEN__
  std::vector<float> coords;
  coords.reserve(points.size() * 2);
  for (const auto &p : points) {
    coords.push_back(p.x);
    coords.push_back(p.y);
  }

  webcanvas__draw_polygon(mId.c_str(), coords.data(),
                          static_cast<int>(points.size()), strokeColor.c_str(),
                          lineWidth, fillColor.c_str());
#else
  (void)points;
  (void)strokeColor;
  (void)lineWidth;
  (void)fillColor;
#endif
}

} // namespace cse498